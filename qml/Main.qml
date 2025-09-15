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
            background: Rectangle {
                color: ft.isConnected ? "green" : "red"
                radius: 6
            }
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
            enabled: ft.isConnected
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

        // Чтение 4 байт
        Button {
            text: "Read 4 bytes"
            enabled: ft.isConnected
            onClicked: {
                let addr = parseInt(ft.slaveAddress)
                let data = ft.readMem(addr, 0, 4)
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
        onLogMessage: {
            logArea.append(msg + "\n")
        }
        onErrorOccurred: {
            logArea.append("FT4222 Error: " + msg + "\n")
        }
    }
}
