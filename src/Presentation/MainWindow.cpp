#include "MainWindow.h"

#include <QDockWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QtCharts/QtCharts>
#include <QSettings>

#include "Infrastructure/Interpolation/LinearInterpolator.h"
#include "Infrastructure/Interpolation/AdditionalInterpolators.h"
#include "Application/UseCases/InterpolationService.h"
#include "Application/UseCases/PointGenerationService.h"

// Types are in global namespace now based on previous findings, 
// using QChartView, QChart etc directly.

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Interpolation App");
    resize(1200, 800);
    
    // Dark Theme
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(darkPalette);
    qApp->setStyle("Fusion");

    setupCharts();
    setupDock();
    
    // Restore settings
    QSettings settings("MyCompany", "InterpolationApp");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    // User requested: "generate points only one time, only when the user wants".
    // So we do NOT call generatePoints() here initially.
}

MainWindow::~MainWindow() {
    QSettings settings("MyCompany", "InterpolationApp");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::setupCharts() {
    chart = new QChart();
    chart->setTitle("Interpolation Result");
    chart->setTheme(QChart::ChartThemeDark);
    
    originalSeries = new QScatterSeries();
    originalSeries->setName("Original Points");
    originalSeries->setMarkerSize(10.0);
    originalSeries->setColor(Qt::red);
    
    interpolatedSeries = new QLineSeries();
    interpolatedSeries->setName("Interpolated");
    interpolatedSeries->setColor(Qt::blue);
    
    chart->addSeries(originalSeries);
    chart->addSeries(interpolatedSeries);
    
    chart->createDefaultAxes();
    
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    setCentralWidget(chartView);
}

void MainWindow::setupDock() {
    QDockWidget *dock = new QDockWidget("Controls", this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    QWidget *multiWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(multiWidget);
    
    // Points Generation
    layout->addWidget(new QLabel("Random Points:"));
    pointsSpin = new QSpinBox();
    pointsSpin->setRange(2, 100000);
    pointsSpin->setValue(10);
    layout->addWidget(pointsSpin);
    
    QPushButton *genBtn = new QPushButton("Generate");
    connect(genBtn, &QPushButton::clicked, this, &MainWindow::generatePoints);
    layout->addWidget(genBtn);
    
    layout->addSpacing(20);
    
    // Interpolation
    layout->addWidget(new QLabel("Method:"));
    methodCombo = new QComboBox();
    methodCombo->addItems({
        "Linear", 
        "Nearest Neighbor", 
        "Lagrange Polynomial", 
        "Cubic Spline", 
        "Akima Spline",
        "Step (Zero-Order)", 
        "Cosine", 
        "Sinc (Lanczos)", 
        "Moving Average",
        "Catmull-Rom Spline"
    });
    connect(methodCombo, &QComboBox::currentIndexChanged, this, &MainWindow::updateInterpolation);
    layout->addWidget(methodCombo);
    
    layout->addWidget(new QLabel("Steps:"));
    stepsSpin = new QSpinBox();
    stepsSpin->setRange(10, 5000);
    stepsSpin->setValue(100);
    connect(stepsSpin, &QSpinBox::valueChanged, this, &MainWindow::updateInterpolation);
    layout->addWidget(stepsSpin);
    
    layout->addStretch();
    
    dock->setWidget(multiWidget);
    dock->setObjectName("ControlsDock");
    addDockWidget(Qt::RightDockWidgetArea, dock);

    // Time Label
    layout->addSpacing(20);
    timeLabel = new QLabel("Time: N/A");
    layout->addWidget(timeLabel);
}

void MainWindow::generatePoints() {
    currentPoints = Application::UseCases::PointGenerationService::generateRandomPoints(pointsSpin->value(), 0, 100, 0, 100);
    updateInterpolation();
}

void MainWindow::updateInterpolation() {
    // Update Chart Data
    originalSeries->clear();
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    
    for(const auto& p : currentPoints) {
        originalSeries->append(p.x.value, p.y.value);
        if(p.x.value < minX) minX = p.x.value;
        if(p.x.value > maxX) maxX = p.x.value;
        if(p.y.value < minY) minY = p.y.value;
        if(p.y.value > maxY) maxY = p.y.value;
    }
    
    interpolatedSeries->clear();
    
    using namespace Infrastructure::Interpolation;
    using namespace Application::UseCases;
    
    std::expected<std::vector<Domain::Model::Point>, Domain::Ports::DomainError> result;
    int steps = stepsSpin->value();

    auto start = std::chrono::high_resolution_clock::now();
    switch(methodCombo->currentIndex()) {
        case 0: { InterpolationService<LinearInterpolator> s((LinearInterpolator())); result = s.execute(currentPoints, steps); break; }
        case 1: { InterpolationService<NearestNeighborInterpolator> s((NearestNeighborInterpolator())); result = s.execute(currentPoints, steps); break; }
        case 2: { InterpolationService<LagrangeInterpolator> s((LagrangeInterpolator())); result = s.execute(currentPoints, steps); break; }
        case 3: { InterpolationService<CubicSplineInterpolator> s((CubicSplineInterpolator())); result = s.execute(currentPoints, steps); break; }
        case 4: { InterpolationService<AkimaSplineInterpolator> s((AkimaSplineInterpolator())); result = s.execute(currentPoints, steps); break; }
        case 5: { InterpolationService<StepInterpolator> s((StepInterpolator())); result = s.execute(currentPoints, steps); break; }
        case 6: { InterpolationService<CosineInterpolator> s((CosineInterpolator())); result = s.execute(currentPoints, steps); break; }
        case 7: { InterpolationService<SincInterpolator> s((SincInterpolator())); result = s.execute(currentPoints, steps); break; }
        case 8: { InterpolationService<MovingAverageInterpolator> s((MovingAverageInterpolator())); result = s.execute(currentPoints, steps); break; }
        case 9: { InterpolationService<CatmullRomInterpolator> s((CatmullRomInterpolator())); result = s.execute(currentPoints, steps); break; }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    timeLabel->setText(QString("Time: %1 µs").arg(dur));
    
    if (result) {
        for(const auto& p : result.value()) {
            interpolatedSeries->append(p.x.value, p.y.value);
            if(p.y.value < minY) minY = p.y.value;
            if(p.y.value > maxY) maxY = p.y.value;
        }
    }
    
    // Rescale axes
    // Note: QChart automatically manages axes ranges if added? Use createDefaultAxes above.
    // If not, we might need to set them manually or re-call createDefaultAxes (which might duplicate).
    // Accessing axes created by createDefaultAxes:
    auto axesX = chart->axes(Qt::Horizontal);
    auto axesY = chart->axes(Qt::Vertical);
    if (!axesX.empty()) axesX.first()->setRange(minX - 5, maxX + 5);
    if (!axesY.empty()) axesY.first()->setRange(minY - 5, maxY + 5);
}
