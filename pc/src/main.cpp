#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QMainWindow>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("AnonymousMessage");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("AnonymousMessage");
    
    // Apply macOS-like styling
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Create dark palette for macOS-like appearance
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(242, 242, 247));
    darkPalette.setColor(QPalette::WindowText, Qt::black);
    darkPalette.setColor(QPalette::Base, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::AlternateBase, QColor(242, 242, 247));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::black);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::black);
    darkPalette.setColor(QPalette::Button, QColor(242, 242, 247));
    darkPalette.setColor(QPalette::ButtonText, Qt::black);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(0, 122, 255));
    darkPalette.setColor(QPalette::Highlight, QColor(0, 122, 255));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    
    app.setPalette(darkPalette);
    
    // Set application font
    QFont font = app.font();
    font.setFamily("SF Pro Text");  // macOS system font
    font.setPointSize(13);
    app.setFont(font);
    
    // Create and show main window
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}