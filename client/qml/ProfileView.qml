import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: profileView
    background: Rectangle { color: "#ffffff" }

    signal backRequested()

    Component.onCompleted: gymMV.fetchStats()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        RowLayout {
            Layout.fillWidth: true

            Button {
                background: Rectangle { color: "transparent" }
                contentItem: Text {
                    text: "< Назад"
                    font.pixelSize: 16
                    color: "#d97706"
                    font.weight: Font.Medium
                }
                onClicked: profileView.backRequested()
            }

            ColumnLayout {
                spacing: 2
                Text {
                    text: "Профиль"
                    font.pixelSize: 28
                    font.weight: Font.Black
                    color: "#2d241e"
                }
                Rectangle {
                    width: 40
                    height: 3
                    color: "#d97706"
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#e5e5e5"
        }

        Rectangle {
            Layout.fillWidth: true
            height: 70
            radius: 12
            color: "#f9f9f9"
            border.color: "#e5e5e5"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Rectangle {
                    width: 40
                    height: 40
                    radius: 20
                    color: gymMV.isAdmin ? "#fef3c7" : "#e0f2fe"

                    Text {
                        anchors.centerIn: parent
                        text: gymMV.isAdmin ? "А" : "С"
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        color: gymMV.isAdmin ? "#d97706" : "#0369a1"
                    }
                }

                Text {
                    text: gymMV.isAdmin ? "Администратор" : "Студент"
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: "#2d241e"
                    Layout.fillWidth: true
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Часы посещений"
                    font.pixelSize: 15
                    font.weight: Font.Medium
                    color: "#2d241e"
                    Layout.fillWidth: true
                }
                Text {
                    text: gymMV.hoursCount + " / " + gymMV.hoursNeeded
                    font.pixelSize: 14
                    color: gymMV.hoursCount >= gymMV.hoursNeeded ? "#166534" : "#57534e"
                    font.weight: Font.Medium
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 14
                radius: 7
                color: "#f0f0f0"

                Rectangle {
                    width: parent.width * Math.min(
                        gymMV.hoursNeeded > 0
                            ? gymMV.hoursCount / gymMV.hoursNeeded
                            : 0,
                        1.0
                    )
                    height: parent.height
                    radius: parent.radius
                    color: gymMV.hoursCount >= gymMV.hoursNeeded ? "#166534" : "#d97706"
                    Behavior on width { NumberAnimation { duration: 400; easing.type: Easing.OutCubic } }
                }
            }

            Text {
                text: gymMV.hoursCount >= gymMV.hoursNeeded
                    ? "Норма выполнена!"
                    : "Осталось: " + (gymMV.hoursNeeded - gymMV.hoursCount) + " ч."
                font.pixelSize: 12
                color: gymMV.hoursCount >= gymMV.hoursNeeded ? "#166534" : "#78716c"
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#e5e5e5"
        }

        Text {
            text: "Статистика по залам"
            font.pixelSize: 15
            font.weight: Font.Medium
            color: "#2d241e"
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: gymMV.gymStatsList
            spacing: 8
            clip: true

            delegate: Rectangle {
                width: ListView.view.width
                height: 52
                radius: 10
                color: "#f9f9f9"
                border.color: "#e5e5e5"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16

                    Text {
                        text: modelData.name
                        font.pixelSize: 14
                        color: "#2d241e"
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    Text {
                        text: modelData.hours + " ч."
                        font.pixelSize: 14
                        color: "#d97706"
                        font.weight: Font.Bold
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: gymMV.isLoading ? "Загрузка..." : "Нет данных по залам"
                color: "#a8a29e"
                font.pixelSize: 14
                visible: parent.count === 0
            }
        }
    }
}
