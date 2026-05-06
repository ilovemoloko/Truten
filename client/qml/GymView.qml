import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: rootGymView
    background: Rectangle { color: "#ffffff" }

    signal logoutRequested()

    // variables for controlling current day
    property date selectedDate: new Date()
    property int dayOffset: 0
    property Component inputBg: Component {
            Rectangle {
                implicitHeight: 45
                color: "#e2e8f0"
                radius: 5
                border.color: parent.activeFocus ? "#d97706" : "transparent"
                border.width: 1
            }
        }

    Timer { // timer for hidding errors messages
        id: errorTimer
        interval: 2000
        repeat: false
        onTriggered: {
            gymMV.errorMessage = ""
        }
    }

    function updateDate() {
        let d = new Date()
        d.setDate(d.getDate() + dayOffset)
        selectedDate = d
    }

    // JS day (0-Sun, 1-Mon) into C++ day (1-Mon, 7-Sun)
    function getCppDayOfWeek() {
        let day = selectedDate.getDay()
        return day === 0 ? 7 : day
    }

    Component.onCompleted: {
        gymMV.init()
        gymMV.fetchHours()
        gymMV.fetchBookedSlotsIds()
        updateDate()
    }

    Connections {
        target: gymMV
        function onActionSuccess(message) {
            console.log("Success:", message)
        }

        function onErrorMessageChanged() {
                if (gymMV.errorMessage !== "") {
                    errorTimer.restart()
                }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // Header interface
        RowLayout {
            Layout.fillWidth: true

            Button {
                text: "<"
                visible: mainLayout.currentIndex === 1
                background: Rectangle { color: "transparent" }
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
                    text: gymMV.isAdmin ? "Админ" : "Студент"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: "#57534e"
                    horizontalAlignment: Text.AlignRight
                }
                Text {
                    text: "Часы: " + gymMV.hoursCount + " / " + gymMV.hoursNeeded
                    font.pixelSize: 12
                    color: "#78716c"
                    horizontalAlignment: Text.AlignRight
                }
            }

            Button {
                text: "Выйти"
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                background: Rectangle { color: "#fee2e2"; radius: 6 }
                palette.buttonText: "#b91c1c"
                onClicked: {
                    rootGymView.logoutRequested()
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

            // WINDOW 0: Gym list
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                // ADMIN PANEL: Create gym
                RowLayout {
                    Layout.fillWidth: true
                    visible: gymMV.isAdmin
                    spacing: 10

                    TextField {
                        id: newGymNameField
                        Layout.fillWidth: true
                        placeholderText: "Название нового зала"
                        Layout.preferredHeight: 50
                        leftPadding: 16
                        font.pixelSize: 14
                        color: "#2d241e"
                        enabled: !authMV.isLoading

                        background: Rectangle {
                            radius: 8
                            color: "#f5f5f5"
                        }
                    }
                    Button {
                        text: "Создать зал"
                        background: Rectangle { color: "#d97706"; radius: 8 }
                        palette.buttonText: "white"
                        onClicked: {
                            if(newGymNameField.text !== "") {
                                gymMV.createGym(newGymNameField.text)
                                newGymNameField.text = ""
                            }
                        }
                    }
                }

                // ADMIN PANEL: Add new admin (TODO)
                // Button {
                //     text: "Добавить администратора"
                //     Layout.fillWidth: true
                //     visible: gymMV.isAdmin
                //     background: Rectangle { color: "#2d241e"; radius: 8 }
                //     palette.buttonText: "white"
                //     onClicked: console.log("TODO: Открыть модалку ввода ID админа и ID зала")
                // }

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

            // WINDOW 1: Slots list
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                // ADMIN PANEL: Create slot
                RowLayout {
                    id: adminRow
                    Layout.fillWidth: true
                    visible: gymMV.isAdmin
                    spacing: 10

                    TextField {
                        id: slotStart
                        placeholderText: "Начало"
                        text: "10:00"
                        inputMask: "99:00"
                        Layout.fillWidth: true
                        font.pixelSize: 16
                        color: "#2d241e"
                        horizontalAlignment: Text.AlignHCenter
                        leftPadding: 0
                        background: Loader { sourceComponent: rootGymView.inputBg }
                    }

                    TextField {
                        id: slotEnd
                        placeholderText: "Конец"
                        text: "11:30"
                        inputMask: "99:00"
                        Layout.fillWidth: true
                        font.pixelSize: 16
                        color: "#2d241e"
                        horizontalAlignment: Text.AlignHCenter
                        leftPadding: 0
                        background: Loader { sourceComponent: rootGymView.inputBg }
                    }

                    TextField {
                        id: slotCap
                        placeholderText: "Мест"
                        Layout.preferredWidth: 65
                        font.pixelSize: 14
                        color: "#2d241e"
                        horizontalAlignment: Text.AlignHCenter
                        validator: IntValidator { bottom: 1; top: 99 }
                        background: Loader { sourceComponent: rootGymView.inputBg }
                    }

                    Button {
                        id: addSlotBtn
                        Layout.preferredWidth: 45
                        Layout.preferredHeight: 45

                        background: Rectangle {
                            color: addSlotBtn.pressed ? "#b45309" : "#d97706"
                            radius: 10
                        }

                        contentItem: Text {
                            text: "+"
                            font.pixelSize: 24
                            color: "#ffffff"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (slotStart.text !== "00:00" && slotCap.text !== "") {
                                let sd = new Date(selectedDate)
                                let startParts = slotStart.text.split(":")
                                sd.setHours(parseInt(startParts[0]), parseInt(startParts[1]), 0)

                                let ed = new Date(selectedDate)
                                let endParts = slotEnd.text.split(":")
                                ed.setHours(parseInt(endParts[0]), parseInt(endParts[1]), 0)

                                gymMV.createSlot(sd, ed, parseInt(slotCap.text))
                            }
                        }
                    }
                }

                // Days scrolbar
                RowLayout {
                    Layout.fillWidth: true
                    Button {
                        text: "< Назад"
                        enabled: dayOffset > 0
                        onClicked: { dayOffset--; updateDate() }
                        Layout.fillWidth: true
                        font.pixelSize: 14
                        background: Rectangle {
                            radius: 4
                            color: "#2d241e"
                        }
                    }
                    Text {
                        text: Qt.formatDate(selectedDate, "dd MMMM")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        font.bold: true
                        font.pixelSize: 16
                    }
                    Button {
                        text: "Вперед >"
                        enabled: dayOffset < 7
                        onClicked: { dayOffset++; updateDate() }
                        Layout.fillWidth: true
                        font.pixelSize: 14
                        background: Rectangle {
                            radius: 4
                            color: "#2d241e"
                        }
                    }
                }

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
                    id: slotsListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    // Model for current day
                    model: gymMV.getDayModel(getCppDayOfWeek())
                    spacing: 10
                    clip: true

                    delegate: Rectangle {
                        id: slotDelegate
                        width: ListView.view.width
                        // More space for admin features
                        height: gymMV.isAdmin ? implicitHeight : 90
                        implicitHeight: contentCol.implicitHeight + 30
                        radius: 12
                        border.color: "#374151"
                        border.width: 1

                        property bool isBooked: false
                        property bool isQueued: false
                        property bool isFull: model.participantsCount >= model.capacity

                        Connections {
                            target: gymMV
                            function onBookedSlotsChanged() {
                                isBooked = gymMV.isSlotBooked(model.slotId)
                                isQueued = gymMV.isSlotQueued(model.slotId)
                            }
                        }
                        Component.onCompleted: {
                            isBooked = gymMV.isSlotBooked(model.slotId)
                            isQueued = gymMV.isSlotQueued(model.slotId)
                        }

                        // green=booked, yellow=queued, red=full
                        color: isBooked ? "#dcfce7" : (isQueued ? "#fef9c3" : (isFull ? "#fee2e2" : "transparent"))

                        ColumnLayout {
                            id: contentCol
                            anchors.fill: parent
                            anchors.margins: 15
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true
                                ColumnLayout {
                                    spacing: 5
                                    Text {
                                        text: Qt.formatDateTime(model.startTime, "HH:mm") + " - " + Qt.formatDateTime(model.endTime, "HH:mm")
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
                                        text: "Занято: " + model.participantsCount + " / " + model.capacity
                                        font.pixelSize: 13
                                        color: "#57534e"
                                        horizontalAlignment: Text.AlignRight
                                        Layout.alignment: Qt.AlignRight
                                    }


                                    // new button: записаться/встать в очередь/выйти из очереди/отменить
                                    Button {
                                        id: actionBtn
                                        visible: !gymMV.isAdmin
                                        Layout.preferredWidth: 150
                                        Layout.preferredHeight: 35

                                        property string btnText: {
                                            if (slotDelegate.isBooked) return "Отменить"
                                            if (slotDelegate.isQueued) return "Выйти из очереди"
                                            if (slotDelegate.isFull) return "Встать в очередь"
                                            return "Записаться"
                                        }
                                        property color btnColor: {
                                            if (slotDelegate.isBooked) return "#f5f5f5"
                                            if (slotDelegate.isQueued) return "#fef08a"
                                            if (slotDelegate.isFull) return "#fed7aa"
                                            return "#f5f5f5"
                                        }
                                        property color btnTextColor: {
                                            if (slotDelegate.isQueued) return "#92400e"
                                            if (slotDelegate.isFull) return "#9a3412"
                                            return "#2d241e"
                                        }

                                        text: btnText
                                        background: Rectangle {
                                            radius: 8
                                            color: parent.pressed ? Qt.darker(actionBtn.btnColor, 1.05) : actionBtn.btnColor
                                        }
                                        contentItem: Text {
                                            text: parent.text
                                            color: actionBtn.btnTextColor
                                            font.weight: Font.Bold
                                            font.pixelSize: 12
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        onClicked: {
                                            if (slotDelegate.isBooked) {
                                                gymMV.cancelBooking(model.slotId)
                                            } else if (slotDelegate.isQueued) {
                                                gymMV.leaveQueue(model.slotId)
                                            } else if (slotDelegate.isFull) {
                                                gymMV.joinQueue(model.slotId)
                                            } else {
                                                gymMV.bookSlot(model.slotId)
                                            }
                                        }
                                    }
                                }
                            }

                            // Admins slot interface
                            ColumnLayout {
                                visible: gymMV.isAdmin
                                Layout.fillWidth: true
                                spacing: 5

                                Rectangle { Layout.fillWidth: true; height: 1; color: "#e5e5e5" } // Разделитель

                                Text { text: "Записанные студенты:"; font.bold: true; font.pixelSize: 16 }

                                // Students info list
                                ColumnLayout {
                                    spacing: 5
                                    Layout.fillWidth: true

                                    Repeater {
                                        model: (typeof participants !== "undefined") ? participants : []

                                        delegate: ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 5

                                            property bool showHoursInput: false

                                            RowLayout {
                                                Layout.fillWidth: true
                                                Text {
                                                    text: "• " + modelData.userName + " (ID: " + modelData.userId + ")"
                                                    font.pixelSize: 12
                                                    color: "#57534e"
                                                    Layout.fillWidth: true
                                                }
                                                Button {
                                                    text: "Добавить часы"
                                                    visible: !showHoursInput
                                                    Layout.preferredHeight: 25
                                                    background: Rectangle {
                                                        radius: 4
                                                        color: parent.pressed ? "#d4d4d4" : "#f5f5f5"
                                                    }
                                                    contentItem: Text {
                                                        text: parent.text
                                                        color: "#57534e"
                                                        font.weight: Font.Medium
                                                        horizontalAlignment: Text.AlignHCenter
                                                        verticalAlignment: Text.AlignVCenter
                                                    }
                                                    onClicked: showHoursInput = true
                                                }
                                            }

                                            // Скрытое поле ввода, появляется при нажатии на "Добавить часы"
                                            RowLayout {
                                                visible: showHoursInput
                                                Layout.fillWidth: true

                                                TextField {
                                                    id: hoursInput
                                                    placeholderText: "Кол-во часов"
                                                    validator: IntValidator { bottom: 1 }
                                                    Layout.preferredWidth: 50
                                                    Layout.preferredHeight: 30
                                                    Layout.fillWidth: true
                                                    leftPadding: 16
                                                    font.pixelSize: 14
                                                    color: "#2d241e"

                                                    background: Rectangle {
                                                        radius: 4
                                                        color: "#f5f5f5"
                                                    }
                                                }

                                                Button {
                                                    text: "ОК"
                                                    Layout.preferredHeight: 30
                                                    background: Rectangle { color: "#dcfce7"; radius: 4 }
                                                    palette.buttonText: "#166534"
                                                    onClicked: {
                                                        if (hoursInput.text !== "") {
                                                            // Вызываем C++ методы
                                                            gymMV.addHours(parseInt(hoursInput.text), modelData.userId)
                                                            // gymMV.cancelBookingForUser(model.slotId, modelData.userId)

                                                            showHoursInput = false
                                                            hoursInput.text = ""
                                                        }
                                                    }
                                                }

                                                Button {
                                                    text: "Отмена"
                                                    Layout.preferredHeight: 30
                                                    background: Rectangle { color: "#fee2e2"; radius: 4 }
                                                    palette.buttonText: "#b91c1c"
                                                    onClicked: showHoursInput = false
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        visible: model.participantsCount === 0
                                        text: "Пока никого нет"
                                        font.pixelSize: 12; color: "#a8a29e"
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 5
                                    visible: model.queueCount > 0

                                    Rectangle { Layout.fillWidth: true; height: 1; color: "#e5e5e5" }

                                    RowLayout {
                                        spacing: 6
                                        Text {
                                            text: "Очередь:"
                                            font.bold: true
                                            font.pixelSize: 16
                                        }
                                        Rectangle {
                                            width: queueBadge.implicitWidth + 10
                                            height: 20
                                            radius: 10
                                            color: "#fed7aa"
                                            Text {
                                                id: queueBadge
                                                anchors.centerIn: parent
                                                text: model.queueCount
                                                font.pixelSize: 12
                                                font.weight: Font.Bold
                                                color: "#9a3412"
                                            }
                                        }
                                    }

                                    Repeater {
                                        model: (typeof slotQueue !== "undefined") ? slotQueue : []
                                        delegate: Text {
                                            text: "• " + modelData.userName + " (ID: " + modelData.userId + ")"
                                            font.pixelSize: 12
                                            color: "#92400e"
                                            Layout.fillWidth: true
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Layout.topMargin: 5
                                    Button {
                                        text: "Удалить слот"
                                        background: Rectangle { color: "#fee2e2"; radius: 6 }
                                        palette.buttonText: "#b91c1c"
                                        onClicked: gymMV.removeSlot(model.slotId)
                                    }
                                    Item { Layout.fillWidth: true }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
