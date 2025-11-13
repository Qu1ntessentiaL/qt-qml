import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 900
    height: 650
    title: "COM-порт мониторинг"
    
    property string messageHistory: ""
    property int bytesSent: 0
    property int bytesReceived: 0

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#f5f5f5" }
            GradientStop { position: 1.0; color: "#e0e0e0" }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // Заголовок
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "#2196F3"
            radius: 8
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 0
                verticalOffset: 2
                radius: 8
                samples: 16
                color: "#80000000"
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    color: serialManager.isOpen ? "#4CAF50" : "#F44336"
                    
                    SequentialAnimation on opacity {
                        running: serialManager.isOpen
                        loops: Animation.Infinite
                        NumberAnimation { from: 1.0; to: 0.3; duration: 800 }
                        NumberAnimation { from: 0.3; to: 1.0; duration: 800 }
                    }
                }

                Text {
                    text: "COM-порт мониторинг"
                    font.pixelSize: 24
                    font.bold: true
                    color: "white"
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: serialManager.isOpen ? "Подключено" : "Отключено"
                    font.pixelSize: 14
                    color: "white"
                    opacity: 0.9
                }
            }
        }

        // Секция настроек подключения
        Rectangle {
            Layout.fillWidth: true
            height: 140
            color: "white"
            radius: 8
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 0
                verticalOffset: 1
                radius: 4
                samples: 8
                color: "#40000000"
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                Text {
                    text: "Настройки подключения"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#333333"
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 15

                    ColumnLayout {
                        spacing: 5
                        Text {
                            text: "COM-порт:"
                            font.pixelSize: 12
                            color: "#666666"
                        }
                        ComboBox {
                            id: portSelector
                            Layout.preferredWidth: 150
                            model: serialManager.ports
                            enabled: !serialManager.isOpen
                            onCurrentTextChanged: serialManager.portName = currentText
                        }
                    }

                    ColumnLayout {
                        spacing: 5
                        Text {
                            text: "Скорость (baud):"
                            font.pixelSize: 12
                            color: "#666666"
                        }
                        ComboBox {
                            id: baudRateSelector
                            Layout.preferredWidth: 150
                            model: ["9600", "19200", "38400", "57600", "115200"]
                            currentIndex: 4
                            enabled: !serialManager.isOpen
                            onCurrentTextChanged: serialManager.baudRate = parseInt(currentText)
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        id: openButton
                        Layout.preferredWidth: 150
                        Layout.preferredHeight: 50
                        text: serialManager.isOpen ? "Закрыть порт" : "Открыть порт"
                        font.pixelSize: 14
                        font.bold: true
                        
                        background: Rectangle {
                            color: serialManager.isOpen ? "#F44336" : "#4CAF50"
                            radius: 6
                            
                            Rectangle {
                                anchors.fill: parent
                                color: "white"
                                opacity: openButton.pressed ? 0.3 : (openButton.hovered ? 0.1 : 0)
                                radius: 6
                            }
                        }
                        
                        contentItem: Text {
                            text: openButton.text
                            font: openButton.font
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        onClicked: {
                            if (serialManager.isOpen)
                                serialManager.closePort()
                            else
                                serialManager.openPort()
                        }
                    }
                }
            }
        }

        // Секция лога сообщений
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            radius: 8
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 0
                verticalOffset: 1
                radius: 4
                samples: 8
                color: "#40000000"
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        text: "Лог сообщений"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#333333"
                    }

                    Item { Layout.fillWidth: true }

                    Switch {
                        id: hexModeSwitch
                        text: "HEX режим"
                    }

                    Button {
                        text: "Очистить"
                        onClicked: {
                            messageHistory = ""
                            logArea.text = ""
                            bytesSent = 0
                            bytesReceived = 0
                        }
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    TextArea {
                        id: logArea
                        readOnly: true
                        wrapMode: TextArea.Wrap
                        font.family: "Courier New"
                        font.pixelSize: 12
                        selectByMouse: true
                        background: Rectangle {
                            color: "#fafafa"
                            border.color: "#e0e0e0"
                            border.width: 1
                            radius: 4
                        }
                    }
                }
            }
        }

        // Секция отправки данных
        Rectangle {
            Layout.fillWidth: true
            height: 120
            color: "white"
            radius: 8
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 0
                verticalOffset: 1
                radius: 4
                samples: 8
                color: "#40000000"
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                Text {
                    text: "Отправка данных"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#333333"
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    TextField {
                        id: inputField
                        Layout.fillWidth: true
                        placeholderText: "Введите сообщение..."
                        font.pixelSize: 12
                        enabled: serialManager.isOpen
                        
                        background: Rectangle {
                            color: inputField.enabled ? "white" : "#f5f5f5"
                            border.color: inputField.activeFocus ? "#2196F3" : "#e0e0e0"
                            border.width: 2
                            radius: 4
                        }
                        
                        Keys.onReturnPressed: sendButton.clicked()
                    }

                    Button {
                        id: sendButton
                        text: "Отправить"
                        Layout.preferredWidth: 120
                        font.pixelSize: 12
                        enabled: serialManager.isOpen && inputField.text.length > 0
                        
                        background: Rectangle {
                            color: sendButton.enabled ? "#2196F3" : "#BDBDBD"
                            radius: 4
                            
                            Rectangle {
                                anchors.fill: parent
                                color: "white"
                                opacity: sendButton.pressed ? 0.3 : (sendButton.hovered ? 0.1 : 0)
                                radius: 4
                            }
                        }
                        
                        contentItem: Text {
                            text: sendButton.text
                            font: sendButton.font
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        onClicked: {
                            if (inputField.text.length > 0) {
                                var timestamp = Qt.formatDateTime(new Date(), "hh:mm:ss.zzz")
                                var message = inputField.text
                                
                                serialManager.sendData(message)
                                bytesSent += message.length
                                
                                var displayMsg = hexModeSwitch.checked ? 
                                    stringToHex(message) : message
                                messageHistory += "[" + timestamp + "] TX: " + displayMsg + "\n"
                                logArea.text = messageHistory
                                logArea.cursorPosition = logArea.length
                                
                                inputField.clear()
                            }
                        }
                    }
                }
            }
        }

        // Статистика
        Rectangle {
            Layout.fillWidth: true
            height: 50
            color: "white"
            radius: 8
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 0
                verticalOffset: 1
                radius: 4
                samples: 8
                color: "#40000000"
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 20

                Text {
                    text: "Статистика:"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#666666"
                }

                Rectangle {
                    width: 2
                    height: 30
                    color: "#e0e0e0"
                }

                Text {
                    text: "Отправлено: " + bytesSent + " байт"
                    font.pixelSize: 12
                    color: "#4CAF50"
                }

                Rectangle {
                    width: 2
                    height: 30
                    color: "#e0e0e0"
                }

                Text {
                    text: "Получено: " + bytesReceived + " байт"
                    font.pixelSize: 12
                    color: "#2196F3"
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: "Порт: " + (serialManager.portName || "не выбран")
                    font.pixelSize: 12
                    color: "#666666"
                }
            }
        }
    }

    // Эффект тени (импортируем)
    component DropShadow: Item {
        property int horizontalOffset: 0
        property int verticalOffset: 0
        property int radius: 0
        property int samples: 0
        property color color: "transparent"
    }

    function stringToHex(str) {
        var hex = ""
        for (var i = 0; i < str.length; i++) {
            var code = str.charCodeAt(i)
            hex += ("0" + code.toString(16).toUpperCase()).slice(-2) + " "
        }
        return hex.trim()
    }

    Connections {
        target: serialManager
        
        function onReceivedDataChanged() {
            var timestamp = Qt.formatDateTime(new Date(), "hh:mm:ss.zzz")
            var data = serialManager.receivedData
            
            if (data.length > 0) {
                bytesReceived += data.length
                
                var displayData = hexModeSwitch.checked ? 
                    stringToHex(data) : data
                messageHistory += "[" + timestamp + "] RX: " + displayData + "\n"
                logArea.text = messageHistory
                logArea.cursorPosition = logArea.length
            }
        }
        
        function onErrorOccurred(msg) {
            var timestamp = Qt.formatDateTime(new Date(), "hh:mm:ss.zzz")
            messageHistory += "[" + timestamp + "] ERROR: " + msg + "\n"
            logArea.text = messageHistory
            logArea.cursorPosition = logArea.length
        }
    }
}
