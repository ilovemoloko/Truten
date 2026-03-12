import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root
    width: 400
    height: 500
    visible: true

    Connections {
        target: authVM
        function onAuthSuccess() {
            console.log("Токен получен! Можно переключать экран.")
            // Здесь будет логика смены экрана
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 15
        width: 250

        Text {
            text: "Вход в систему"
            font.pixelSize: 24
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 20
        }

        TextField {
            id: emailInput
            placeholderText: "Email"
            Layout.fillWidth: true
            enabled: !authVM.isLoading
        }

        TextField {
            id: passwordInput
            placeholderText: "Пароль"
            echoMode: TextInput.Password
            Layout.fillWidth: true
            enabled: !authVM.isLoading
        }

        Text {
            text: authVM.errorMessage
            color: "red"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            visible: authVM.errorMessage !== ""
        }

        Button {
            text: authVM.isLoading ?
            Layout.fillWidth: true
            enabled: !authVM.isLoading && emailInput.text !== "" && passwordInput.text !== ""

            onClicked: {
                authVM.login(emailInput.text, passwordInput.text)
            }
        }

        Button {
            text: "Регистрация"
            Layout.fillWidth: true
            enabled: !authVM.isLoading
            flat: true

            onClicked: {
                authVM.createAccount(emailInput.text, passwordInput.text)
            }
        }
    }
}
