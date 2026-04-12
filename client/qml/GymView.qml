import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: rootGymView
    background: Rectangle { color: "#ffffff" }

    Component.onCompleted: {
        gymMV.init()
    }

    Connections {
        target: gymMV
        function onActionSuccess(message) {
            console.log("Success:", message)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: "<"
                text: "<"
                visible: mainLayout.currentIndex === 1
                background: Rectangle {
                    color: "transparent"
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 24
                    font.weight: Font.Bold
                    color: "#d97706"
                }
                onClicked: mainLayout.currentIndex = 0
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: mainLayout.currentIndex === 0 ? "Спортивные залы" : "Выбор слота"
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

            ColumnLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 2
                Text {
                    text: gymMV.userName !== "" ? gymMV.userName : "Студент"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: "#57534e"
                    horizontalAlignment: Text.AlignRight
                }
                Text {
                    text: "Часы: " + gymMV.visitCount + " / " + gymMV.visitsNeeded
                    font.pixelSize: 12
                    color: "#78716c"
                    horizontalAlignment: Text.AlignRight
                }
            }
        }

        Text {
            text: gymMV.isLoading ? "Загрузка данных..." : gymMV.errorMessage
            color: gymMV.isLoading ? "#d97706" : "#b91c1c"
            font.pixelSize: 14
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            visible: gymMV.isLoading || gymMV.errorMessage !== ""
        }

        StackLayout {
            id: mainLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 0 // 0 gyms, 1 slots

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                Button {
                    text: "Обновить залы"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    background: Rectangle {
                        radius: 10
                        color: parent.pressed ? "#d4d4d4" : "#f5f5f5"
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#57534e"
                        font.weight: Font.Medium
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: gymMV.init()
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: gymMV.gyms
                    spacing: 10
                    clip: true

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 70
                        radius: 12
                        color: "transparent"
                        border.color: "#e5e5e5"
                        border.width: 1

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            text: modelData.name
                            font.pixelSize: 18
                            font.weight: Font.Medium
                            color: "#2d241e"
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 20
                            text: ">"
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            color: "#d97706"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                gymMV.selectGym(modelData.id)
                                mainLayout.currentIndex = 1
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                Button {
                    text: "Обновить слоты"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    background: Rectangle {
                        radius: 10
                        color: parent.pressed ? "#d4d4d4" : "#f5f5f5"
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#57534e"
                        font.weight: Font.Medium
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: gymMV.loadSlots(gymMV.selectedGymId)
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: gymMV.Slots
                    spacing: 10
                    clip: true

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 90
                        radius: 12
                        color: "transparent"
                        border.color: "#e5e5e5"
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 15

                            ColumnLayout {
                                spacing: 5
                                Text {
                                    text: modelData.dayOfWeek + ", " + modelData.dateDisplay
                                    font.pixelSize: 14
                                    color: "#d97706"
                                    font.weight: Font.Bold
                                }
                                Text {
                                    text: modelData.timeDisplay
                                    font.pixelSize: 22
                                    font.weight: Font.Black
                                    color: "#2d241e"
                                }
                            }

                            Item { Layout.fillWidth: true }

                            ColumnLayout {
                                Layout.alignment: Qt.AlignRight
                                spacing: 8

                                Text {
                                    text: "Занято: " + modelData.participantsCount + " / " + modelData.capacity
                                    font.pixelSize: 13
                                    color: "#57534e"
                                    horizontalAlignment: Text.AlignRight
                                    Layout.alignment: Qt.AlignRight
                                }

                                Button {
                                    text: "Записаться"
                                    Layout.preferredWidth: 120
                                    Layout.preferredHeight: 35
                                    enabled: modelData.participantsCount < modelData.capacity

                                    background: Rectangle {
                                        radius: 8
                                        color: parent.enabled ? (parent.pressed ? "#d4d4d4" : "#f5f5f5") : "#f9fafb"
                                        border.color: parent.enabled ? "transparent" : "#e5e5e5"
                                    }

                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.enabled ? "#2d241e" : "#a8a29e"
                                        font.weight: Font.Bold
                                        font.pixelSize: 13
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    onClicked: {
                                        gymMV.bookSlot(modelData.id)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
