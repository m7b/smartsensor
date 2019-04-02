
var host = 'iot.eclipse.org'; //'iot.eclipse.org'; //'bierfass';
var port = 80; //80; //9001;
var topic = 'WS/RWS/#';
var useTLS = false;
var cleansession = true;
var mqtt;
var reconnectTimeout = 2000;
var srcLevelChart = new Array;
var srcLevel = new Array;
var dstLevelChart = new Array;
var dstLevel = new Array;


function MQTTconnect() {
//console.log("myPort: " + my_port);
    if (typeof path == "undefined") {
        path = '/mqtt';
    }
    mqtt = new Paho.Client(host, port, path, "mqtt_panel" + parseInt(Math.random() * 100, 10));
    var options = {
        timeout: 3,
        useSSL: useTLS,
        cleanSession: cleansession,
        onSuccess: onConnect,
        onFailure: function (message) {
            $('#status').html("Connection failed: " + message.errorMessage + "Retrying...");
            setTimeout(MQTTconnect, reconnectTimeout);
        }
    };
    mqtt.onConnectionLost = onConnectionLost;
    mqtt.onMessageArrived = onMessageArrived;
    console.log("Host: "+ host + ", Port: " + port + ", Path: " + path + " TLS: " + useTLS);
    mqtt.connect(options);
};


function onConnect() {
    $('#status').html('Connected to ' + host + ':' + port + path);
    mqtt.subscribe(topic, {qos: 0});
    $('#topic').html(topic);
};


function onConnectionLost(responseObject) {
    setTimeout(MQTTconnect, reconnectTimeout);
    $('#status').html("Connection lost: " + responseObject.errorMessage + ". Reconnecting...");
};


function onMessageArrived(message) {
    var topic = message.destinationName;
    var payload = message.payloadString;
    //console.log("Topic: " + topic + ", Message payload: " + payload);
    $('#message').html(topic + ', ' + payload);
    var timestamp = Math.round((new Date()).getTime() / 1000);
    switch (topic) {
        case 'WS/RWS/EG/BarrelSrc/Status': 
            $('#SrcStatusValue').html('(Payload value: ' + payload + ')');
            $('#SrcStatusLabel').text(payload + '');
            break;
        case 'WS/RWS/EG/BarrelSrc/Lvl/local_timestamp': 
            $('#value0').html('(Payload value: ' + payload + ')');
            $('#label0').text(payload + '');
            break;
        case 'WS/RWS/EG/BarrelSrc/Lvl/raw_cm': 
            $('#value1').html('(Payload value: ' + payload + ')');
            $('#label1').text(payload + 'cm');
            break;
        case 'WS/RWS/EG/BarrelSrc/Lvl/cm': 
            $('#value2').html('(Payload value: ' + payload + ')');
            $('#label2').text(payload + 'cm');
            var entry = new Array();
            entry.push(timestamp);
            entry.push(parseInt(payload));
            srcLevelChart.push(entry);
            // Show only 20 values
            if (srcLevelChart.length >= 20) {
                srcLevelChart.shift()
            }
            var livingTempPlot = $.jqplot ('srcLevelChart', [srcLevelChart], {
                axesDefaults: {
                    labelRenderer: $.jqplot.CanvasAxisLabelRenderer,
                    tickOptions: {
                        showMark: false,
                        showGridline: false,
                        show: false,
                        showLabel: false,
                    }
                    },
                grid: {
                    gridLineColor: '#FFFFFF',
                    borderWidth: 0,
                    shadow: false,
                },
                seriesDefaults: {
                    rendererOptions: {
                        smooth: true
                    },
                    showMarker: false,
                    lineWidth: 2,
                    },
                    axes: {
                    xaxis: {
                        renderer:$.jqplot.DateAxisRenderer,
                        tickOptions:{
                        formatString:'%T'
                        },
                        pad: 0
                    },
                    yaxis: {
                    }
                }
            });
            break;
        case 'WS/RWS/EG/BarrelSrc/Lvl/percent': 
            $('#value3').html('(Payload value: ' + payload + ')');
            $('#label3').text(payload + '%');
            $('#barSrc').css('width', payload + '%');
            $('#barSrc').text(payload + '%');
            srcLevel.push(parseInt(payload));
            if (srcLevel.length >= 20) {
                srcLevel.shift()
            }
            $('.srcLevelSparkline').sparkline(srcLevel, {
                type: 'line',
                width: '160',
                height: '40'});
            break;
        case 'WS/RWS/EG/BarrelSrc/FunctionModeReq':
            $('#FunctionModeRequest').html('(Payload value: ' + payload + ')');
            $('#FunctionModeRequestLabel').removeClass('').addClass('label-default');
            switch (payload){
                case '0':
                    $('#FunctionModeRequestLabel').text('Permanent Meassure'); break;
                case '1':
                    $('#FunctionModeRequestLabel').text('Interval Meassure 5 Sec'); break;
                case '2':
                    $('#FunctionModeRequestLabel').text('Interval Meassure 10 Sec'); break;
                case '3':
                    $('#FunctionModeRequestLabel').text('Interval Meassure 5 Min'); break;
                case '4':
                    $('#FunctionModeRequestLabel').text('Deep Sleep Meassure 20 Sec'); break;
                case '5':
                    $('#FunctionModeRequestLabel').text('Deep Sleep Meassure 5 Min'); break;
                default:
                    $('#FunctionModeRequestLabel').text('Mode not defined'); break;
                    console.log('Error: No Function Mode for this payload.');
                    break;
            }
            break;
        case 'WS/RWS/EG/BarrelSrc/FunctionModeAck':
            $('#FunctionModeAcknowledge').html('(Payload value: ' + payload + ')');
            $('#FunctionModeAcknowledgeLabel').removeClass('').addClass('label-default');
            switch (payload){
                case '0':
                    $('#FunctionModeAcknowledgeLabel').text('Permanent Meassure'); break;
                case '1':
                    $('#FunctionModeAcknowledgeLabel').text('Interval Meassure 5 Sec'); break;
                case '2':
                    $('#FunctionModeAcknowledgeLabel').text('Interval Meassure 10 Sec'); break;
                case '3':
                    $('#FunctionModeAcknowledgeLabel').text('Interval Meassure 5 Min'); break;
                case '4':
                    $('#FunctionModeAcknowledgeLabel').text('Deep Sleep Meassure 20 Sec'); break;
                case '5':
                    $('#FunctionModeAcknowledgeLabel').text('Deep Sleep Meassure 5 Min'); break;
                default:
                    $('#FunctionModeAcknowledgeLabel').text('Mode not defined'); break;
                    console.log('Error: No Function Mode for this payload.');
                    break;
            }
            break;
            
        // DESTINATION BARREL

        case 'WS/RWS/EG/BarrelDst/Status': 
            $('#DstStatusValue').html('(Payload value: ' + payload + ')');
            $('#DstStatusLabel').text(payload + '');
            break;
        case 'WS/RWS/DG/BarrelDst/Lvl/local_timestamp': 
            $('#DstValue0').html('(Payload value: ' + payload + ')');
            $('#DstLabel0').text(payload + '');
            break;
        case 'WS/RWS/DG/BarrelDst/Lvl/raw_cm': 
            $('#DstValue1').html('(Payload value: ' + payload + ')');
            $('#DstLabel1').text(payload + 'cm');
            break;
        case 'WS/RWS/DG/BarrelDst/Lvl/cm': 
            $('#DstValue2').html('(Payload value: ' + payload + ')');
            $('#DstLabel2').text(payload + 'cm');
            var entry = new Array();
            entry.push(timestamp);
            entry.push(parseInt(payload));
            dstLevelChart.push(entry);
            // Show only 20 values
            if (dstLevelChart.length >= 20) {
                dstLevelChart.shift()
            }
            var livingTempPlot = $.jqplot ('dstLevelChart', [dstLevelChart], {
                axesDefaults: {
                    labelRenderer: $.jqplot.CanvasAxisLabelRenderer,
                    tickOptions: {
                        showMark: false,
                        showGridline: false,
                        show: false,
                        showLabel: false,
                    }
                    },
                grid: {
                    gridLineColor: '#FFFFFF',
                    borderWidth: 0,
                    shadow: false,
                },
                seriesDefaults: {
                    rendererOptions: {
                        smooth: true
                    },
                    showMarker: false,
                    lineWidth: 2,
                    },
                    axes: {
                    xaxis: {
                        renderer:$.jqplot.DateAxisRenderer,
                        tickOptions:{
                        formatString:'%T'
                        },
                        pad: 0
                    },
                    yaxis: {
                    }
                }
            });
            break;
        case 'WS/RWS/DG/BarrelDst/Lvl/percent': 
            $('#DstValue3').html('(Payload value: ' + payload + ')');
            $('#DstLabel3').text(payload + '%');
            $('#barDst').css('width', payload + '%');
            $('#barDst').text(payload + '%');
            dstLevel.push(parseInt(payload));
            if (dstLevel.length >= 20) {
                dstLevel.shift()
            }
            $('.dstLevelSparkline').sparkline(dstLevel, {
                type: 'line',
                width: '160',
                height: '40'});
            break;
        case 'WS/RWS/DG/BarrelDst/FunctionModeReq':
            $('#DstFunctionModeRequest').html('(Payload value: ' + payload + ')');
            $('#DstFunctionModeRequestLabel').removeClass('').addClass('label-default');
            switch (payload){
                case '0':
                    $('#DstFunctionModeRequestLabel').text('Permanent Meassure'); break;
                case '1':
                    $('#DstFunctionModeRequestLabel').text('Interval Meassure 5 Sec'); break;
                case '2':
                    $('#DstFunctionModeRequestLabel').text('Interval Meassure 10 Sec'); break;
                case '3':
                    $('#DstFunctionModeRequestLabel').text('Interval Meassure 5 Min'); break;
                case '4':
                    $('#DstFunctionModeRequestLabel').text('Deep Sleep Meassure 20 Sec'); break;
                case '5':
                    $('#DstFunctionModeRequestLabel').text('Deep Sleep Meassure 5 Min'); break;
                default:
                    $('#DstFunctionModeRequestLabel').text('Mode not defined'); break;
                    console.log('Error: No Function Mode for this payload.');
                    break;
            }
            break;
        case 'WS/RWS/DG/BarrelDst/FunctionModeAck':
            $('#DstFunctionModeAcknowledge').html('(Payload value: ' + payload + ')');
            $('#DstFunctionModeAcknowledgeLabel').removeClass('').addClass('label-default');
            switch (payload){
                case '0':
                    $('#DstFunctionModeAcknowledgeLabel').text('Permanent Meassure'); break;
                case '1':
                    $('#DstFunctionModeAcknowledgeLabel').text('Interval Meassure 5 Sec'); break;
                case '2':
                    $('#DstFunctionModeAcknowledgeLabel').text('Interval Meassure 10 Sec'); break;
                case '3':
                    $('#DstFunctionModeAcknowledgeLabel').text('Interval Meassure 5 Min'); break;
                case '4':
                    $('#DstFunctionModeAcknowledgeLabel').text('Deep Sleep Meassure 20 Sec'); break;
                case '5':
                    $('#DstFunctionModeAcknowledgeLabel').text('Deep Sleep Meassure 5 Min'); break;
                default:
                    $('#DstFunctionModeAcknowledgeLabel').text('Mode not defined'); break;
                    console.log('Error: No Function Mode for this payload.');
                    break;
            }
            break;
            
            // CONTROLLER

            case 'WS/RWS/Dashboard/ManualPumpReq': 
                $('#ManualPumpReqValue').html('(Payload value: ' + payload + ')');
                if (payload == '0'){
                    $('#ManualPumpReqLabel').text('Off');
                }
                else{
                    $('#ManualPumpReqLabel').text('On');
                }
                break;

            case 'WS/RWS/Dashboard/ManualPumpAck': 
                $('#ManualPumpAckValue').html('(Payload value: ' + payload + ')');
                if (payload == '0'){
                    $('#ManualPumpAckLabel').text('Off');
                }
                else{
                    $('#ManualPumpAckLabel').text('On');
                }
                break;

            case 'WS/RWS/Dashboard/ManualValveReq': 
                $('#ManualValveReqValue').html('(Payload value: ' + payload + ')');
                if (payload == '0'){
                    $('#ManualValveReqLabel').text('Off');
                }
                else{
                    $('#ManualValveReqLabel').text('On');
                }
                break;

            case 'WS/RWS/Dashboard/ManualValveAck': 
                $('#ManualValveAckValue').html('(Payload value: ' + payload + ')');
                if (payload == '0'){
                    $('#ManualValveAckLabel').text('Off');
                }
                else{
                    $('#ManualValveAckLabel').text('On');
                }
                break;

        default: console.log('Error: Data do not match the MQTT topic.'); break;
    }
};


function myFunction(fct_mode) {
    message = new Paho.Message(fct_mode);
    message.retained = true;
    message.qos = 1;
    message.destinationName = "WS/RWS/EG/BarrelSrc/FunctionModeReq";
    mqtt.send(message);
    message.destinationName = "WS/RWS/DG/BarrelDst/FunctionModeReq";
    mqtt.send(message);
};


function ManualFctPump(fct_mode) {
    message = new Paho.Message(fct_mode);
    message.retained = true;
    message.qos = 1;
    message.destinationName = "WS/RWS/Dashboard/ManualPumpReq";
    mqtt.send(message);
};


function ManualFctValve(fct_mode) {
    message = new Paho.Message(fct_mode);
    message.retained = true;
    message.qos = 1;
    message.destinationName = "WS/RWS/Dashboard/ManualValveReq";
    mqtt.send(message);
};


$(document).ready(function() {
    MQTTconnect();
});