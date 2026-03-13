import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root
    width: 450
    height: 650
    visible: true
    title: "Трутень"
    color: "#ffffff"


    StackView {
            id: mainStack
            anchors.fill: parent
            initialItem: authPage
    }
    Connections {
        target: authVM
        function onAuthSuccess() {
            mainStack.replace("GymView.qml")
        }
    }

    property bool isRegisterMode: false

    Component {
        id: authPage
        Pane{
            background: Rectangle {
                            color: "#ffffff"
                }
            ColumnLayout {
                    anchors.centerIn: parent
                    width: 320
                    spacing: 20

                    ColumnLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 2
                        Text {
                            text: "Трутень"
                            font.pixelSize: 42
                            font.letterSpacing: 4
                            font.weight: Font.Black
                            color: "#2d241e"
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Rectangle {
                            width: 40
                            height: 3
                            color: "#d97706"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }

                    Item { Layout.preferredHeight: 10 }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        radius: 12
                        color: "transparent"
                        border.color: "#e5e5e5"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: isRegisterMode ? "Регистрация" : "Авторизация"
                            font.pixelSize: 16
                            font.weight: Font.Medium
                            color: "#57534e"
                        }
                    }

                    TextField {
                        id: nameInput
                        placeholderText: "Ваше имя"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        leftPadding: 16
                        font.pixelSize: 14
                        color: "#2d241e"
                        visible: isRegisterMode
                        enabled: !authVM.isLoading

                        background: Rectangle {
                            radius: 10
                            color: "#f5f5f5"
                        }
                    }

                    TextField {
                        id: emailInput
                        placeholderText: "Электронная почта"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        leftPadding: 16
                        font.pixelSize: 14
                        color: "#2d241e"
                        enabled: !authVM.isLoading

                        background: Rectangle {
                            radius: 10
                            color: "#f5f5f5"
                        }
                    }

                    TextField {
                        id: passwordInput
                        placeholderText: "Пароль"
                        echoMode: TextInput.Password
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        leftPadding: 16
                        font.pixelSize: 14
                        color: "#2d241e"
                        enabled: !authVM.isLoading

                        background: Rectangle {
                            radius: 10
                            color: "#f5f5f5"
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

                    Item { Layout.preferredHeight: 5 }

                    Button {
                        id: loginButton
                        text: {
                            if (authVM.isLoading) return "Ожидание..."
                            return isRegisterMode ? "Зарегистрироваться" : "Войти в систему"
                        }
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        enabled: !authVM.isLoading && emailInput.text !== "" && passwordInput.text !== ""

                        contentItem: Text {
                            text: loginButton.text
                            color: "#57534e"
                            font.weight: Font.Bold
                            font.pixelSize: 15
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            radius: 10
                            color: loginButton.pressed ? "#d4d4d4" : "#e5e5e5"
                            Behavior on color { ColorAnimation { duration: 100 } }
                        }

                        onClicked: {
                            if (isRegisterMode) {
                                authVM.createAccount(nameInput.text, emailInput.text, passwordInput.text)
                            } else {
                                authVM.login(emailInput.text, passwordInput.text)
                            }
                        }
                    }

                    Button {
                        id: regButton
                        text: isRegisterMode ? "Уже есть аккаунт? Войти" : "Создать новый аккаунт"
                        Layout.fillWidth: true
                        flat: true

                        contentItem: Text {
                            text: regButton.text
                            color: "#78716c"
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignHCenter
                        }

                        onClicked: isRegisterMode = !isRegisterMode
                    }
                }
        }
    }



}
