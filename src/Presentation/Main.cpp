#include <QApplication>
#include "MainWindow.h"

/**
 * @brief Application Entry Point.
 * 
 * @param argc Argument count.
 * @param argv Argument values.
 * @return int Exit code.
 */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.showMaximized(); // User requested "app.showMaximized()"

    return app.exec();
}
