import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Pane {
    id: gymRoot

    background: Rectangle {
        color: "#f8f9fa"
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "Добро пожаловать в зал!"
            font.pixelSize: 24
            font.bold: true
            color: "#2d241e"
            Layout.alignment: Qt.AlignHCenter
        }

        Rectangle {
            width: 200
            height: 200
            radius: 100
            color: "#d97706"
            Layout.alignment: Qt.AlignHCenter

            Text {
                anchors.centerIn: parent
                text: "🏋️"
                font.pixelSize: 80
            }
        }

        Button {
            text: "Назад к авторизации"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                if (typeof mainStack !== "undefined") {
                    mainStack.pop()
                }
            }
        }
    }
}
