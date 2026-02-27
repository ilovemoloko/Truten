#include <QApplication>
#include <QPushButton>
#include "AuthModel.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QNetworkAccessManager *networkManager = new QNetworkAccessManager(&a);

    /* HINT: all models use ONLY one network manager (check BaseModel.h).
     * Thats why we need to initialize 3 models with *networkManager.
     * TODO: main.cpp (button is just a "stub")
     */

    AuthModel *authApi = new AuthModel(networkManager, &a);

    QPushButton button("Hello world!", nullptr);
    button.resize(200, 100);
    button.show();
    return QApplication::exec();
}
