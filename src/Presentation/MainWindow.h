#pragma once

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <vector>
#include "Domain/Model/Point.h"

class QComboBox;
class QSpinBox;
class QLabel;

/**
 * @brief Main Application Window.
 * @details Handles UI setup, event handling, and coordination between services.
 * Inherits from QMainWindow.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructs the main window.
     * @param parent Owning widget, default nullptr.
     */
    explicit MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor. Saves window state.
     */
    ~MainWindow();

private:
    QChartView *chartView;
    QChart *chart;
    QScatterSeries *originalSeries;
    QLineSeries *interpolatedSeries;
    
    QComboBox *methodCombo;
    QSpinBox *stepsSpin;
    QSpinBox *pointsSpin;
    QLabel *timeLabel;
    
    std::vector<Domain::Model::Point> currentPoints;

    /**
     * @brief Initializes QtCharts components.
     */
    void setupCharts();
    
    /**
     * @brief Initializes Dock widget and controls.
     */
    void setupDock();
    
    /**
     * @brief Generates random points using the selected configuration.
     */
    void generatePoints();
    
    /**
     * @brief Runs the selected interpolation strategy and updates the chart.
     */
    void updateInterpolation();
};
