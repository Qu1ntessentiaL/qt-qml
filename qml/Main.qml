import QtQuick
import QtQuick.Controls
import MyLib 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 480
    title: "I2C Demo"

    Ft4222Wrapper {
        id: ft
    }

    Column {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10
        spacing: 10

        // Подключение/отключение устройства
        Button {
            text: ft.isConnected ? "Disconnect" : "Connect"
            /*
            background: Rectangle {
                color: ft.isConnected ? "green" : "red"
                radius: 6
            }
            */
            onClicked: {
                if (ft.isConnected) {
                    ft.disconnectDevice()
                    logArea.text = ""  // очищаем логи
                } else {
                    logArea.text = ""  // очищаем перед новым соединением
                    ft.connectDevice()
                }
            }
        }

        // Сканирование I2C шины
        Button {
            text: "Scan I2C Bus"
            onClicked: ft.scanDevices()
        }

        // Ввод slave-адреса
        TextField {
            id: addressField
            width: 120
            placeholderText: "Slave address"
            text: ft.slaveAddress

            validator: RegularExpressionValidator {
                regularExpression: /0x[0-9A-Fa-f]{0,2}/
            }

            inputMethodHints: Qt.ImhPreferUppercase | Qt.ImhHexadecimal

            onActiveFocusChanged: {
                if (activeFocus && text === "")
                    text = "0x"
            }

            onTextChanged: ft.slaveAddress = text
        }

        ComboBox {
            id: addressWidthSelector
            width: 160
            model: ["Register width: 1 byte", "Register width: 2 bytes"]
            onCurrentIndexChanged: {
                ft.registerAddressWidth = currentIndex + 1
            }
            Component.onCompleted: currentIndex = ft.registerAddressWidth - 1
        }

        // Чтение 4 байт
        Button {
            text: "Read 4 bytes"
            enabled: ft.isConnected
            onClicked: {
                let addr = parseInt(ft.slaveAddress)
                if (isNaN(addr)) {
                    logArea.append("Invalid slave address\n")
                    return
                }
                let data = ft.readMem(addr, 0, 4)
                if (data.length === 0) {
                    return
                }
                logArea.append("Read: " + data + "\n")
            }
        }
    }

    // TextArea для логов
    TextArea {
        id: logArea
        width: parent.width - 20
        height: 200
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 10
        readOnly: true
        wrapMode: Text.Wrap
    }

    Connections {
        target: ft

        function onLogMessage(msg) {
            logArea.append(msg + "\n")
        }

        function onErrorOccurred(msg) {
            logArea.append("FT4222 Error: " + msg + "\n")
        }

        function onRegisterAddressWidthChanged() {
            addressWidthSelector.currentIndex = ft.registerAddressWidth - 1
        }
    }
}
