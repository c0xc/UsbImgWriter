#define DEFINE_GLOBALS
#include "main.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    //app.setApplicationName(PROGRAM);
    //app.setApplicationVersion(APP_VERSION);
    //QString program = QString(PROGRAM).toLower();

    //Initialize QTranslator, load default translation
    //...

    //Load default font
    //Note that Qt no longer ships fonts. Deploy some
    //or include some in AppImage:
    //QFontDatabase: Cannot find font directory $QTDIR/qtbase/lib/fonts.
    int font_id = QFontDatabase::addApplicationFont(":/Roboto-Regular.ttf");

    MainWindow *main = new MainWindow;
    main->show();

    int code = app.exec();
    return code;
}

