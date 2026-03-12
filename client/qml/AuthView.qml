import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root
    width: 450
    height: 600
    visible: true
    title: "Truten"
    color: "#fdfbf7" // Кремовый фон

    ColumnLayout {
        anchors.centerIn: parent
        width: 320
        spacing: 25

        // Заголовок в стиле Эко-тек
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 2
            Text {
                text: "Трутень"
                font.pixelSize: 42
                font.letterSpacing: 4
                font.weight: Font.Black
                color: "#2d241e" // Глубокий кофейный
                Layout.alignment: Qt.AlignHCenter
            }
            Rectangle {
                width: 40
                height: 3
                color: "#d97706" // Акцентная линия
                Layout.alignment: Qt.AlignHCenter
            }
        }

        // Карточка формы
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 360
            radius: 20
            color: "#ffffff"
            // Легкая рамка теплого оттенка
            border.color: "#e7e5e4"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 28
                spacing: 18

                Text {
                    text: "Авторизация"
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: "#57534e"
                    Layout.alignment: Qt.AlignHCenter
                    Layout.bottomMargin: 5
                }

                // Инпуты с мягким фоном
                TextField {
                    id: emailInput
                    placeholderText: "Электронная почта"
                    Layout.fillWidth: true
                    leftPadding: 16
                    topPadding: 14
                    bottomPadding: 14
                    font.pixelSize: 14
                    color: "#2d241e"
                    enabled: !authVM.isLoading
                    background: Rectangle {
                        radius: 10
                        color: "#f5f5f4" // Цвет камня
                        border.color: emailInput.activeFocus ? "#d97706" : "transparent"
                        border.width: 1.5
                    }
                }

                TextField {
                    id: passwordInput
                    placeholderText: "Пароль"
                    echoMode: TextInput.Password
                    Layout.fillWidth: true
                    leftPadding: 16
                    topPadding: 14
                    bottomPadding: 14
                    font.pixelSize: 14
                    color: "#2d241e"
                    enabled: !authVM.isLoading
                    background: Rectangle {
                        radius: 10
                        color: "#f5f5f4"
                        border.color: passwordInput.activeFocus ? "#d97706" : "transparent"
                        border.width: 1.5
                    }
                }

                Text {
                    text: authVM.errorMessage
                    color: "#b91c1c"
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    visible: authVM.errorMessage !== ""
                    horizontalAlignment: Text.AlignHCenter
                }

                Item { Layout.fillHeight: true }

                // Главная кнопка: Темный текст на медовом фоне
                Button {
                    id: loginButton
                    text: authVM.isLoading ? "Ожидание..." : "Войти в систему"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    enabled: !authVM.isLoading && emailInput.text !== "" && passwordInput.text !== ""

                    contentItem: Text {
                        text: loginButton.text
                        // Темный цвет для видимости на светлом фоне
                        color: loginButton.enabled ? "#451a03" : "#78716c"
                        font.weight: Font.Bold
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        radius: 12
                        // Медовые/янтарные оттенки
                        color: loginButton.pressed ? "#f59e0b" : (loginButton.enabled ? "#fbbf24" : "#e7e5e4")

                        Behavior on color { ColorAnimation { duration: 150 } }
                    }

                    onClicked: authVM.login(emailInput.text, passwordInput.text)
                }

                Button {
                    id: regButton
                    text: "Создать новый аккаунт"
                    Layout.fillWidth: true
                    flat: true
                    onClicked: authVM.createAccount(emailInput.text, passwordInput.text)

                    contentItem: Text {
                        text: regButton.text
                        color: regButton.pressed ? "#d97706" : "#78716c"
                        font.pixelSize: 13
                        font.underline: regButton.hovered
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }
}
