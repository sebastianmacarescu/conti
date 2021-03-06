var GUI = function(){
  var parent = this;
  this.mode = 0; //0 - TEST, 1 - MANUAL, 2 - AUTO
  this.commands = {
    _start: "init",
    _stop: "s",
    _auto: "auto",
    _traction: "t",
    _steering: "steer",
    _relay: "relay_serial"
  };
  this.relayState = false;
  this.commandFullStop = ';';
  this.predefinedTestCommands = [
    this.commands._start,
    //this.generateTractionCommand(50),
    this.generateSteeringCommand(100),
    //this.generateTractionCommand(100),
    this.generateSteeringCommand(0),
    //this.generateTractionCommand(0),
    this.generateSteeringCommand(-255),
    //this.generateTractionCommand(100),
    this.generateSteeringCommand(0),
    //this.generateTractionCommand(50),
    this.generateSteeringCommand(150),
    this.commands._stop
  ];
  this.pressedKeys = [];
  this.tractionValue = 50;
  this.tractionIncrementValue = 50;
  this.steeringValue = 255;
  this.GUIElements = {
                   timeSinceConnection: '#timeSinceConnection',
                   timeSinceStarted: '#timeSinceStarted',
                   debuggerConsole: '#debuggerConsole',
                   deviceStatus: '#deviceStatus',
                   timestamp: '#timestamp',
                   timeSinceConnection: '#timeSinceConnection',
                   batteryLevel: '#batteryLevel',
                   traction: '#traction',
                   distance: '#distance',
                   speed: '#speed',
                   camera: "#cam"
                };

  this.timers = {timerSinceStarted:0, timerSinceConnected:0};
  this.socket = new WebSocket("ws://localhost:8080/ws");
  this.socket.onopen = this.changeStatus(1);
  this.socket.onclose = this.changeStatus(0);
  this.updateTimers();
  this.socket.onmessage = function(message) {
    parent.updateData(message);
  }; 
  this.setCommandMode(0);
}
GUI.prototype.changeStatus = function(status) {
    switch(status)
    {
      case 0:
        $(this.GUIElements.deviceStatus).removeClass('connected')
                                        .removeClass('transmitting')
                                        .addClass('disconnected')
                                        .find('p')
                                        .text('Device not connected.');
      break;
      case 1:
        $(this.GUIElements.deviceStatus).removeClass('disconnected')
                                        .addClass('connected')
                                        .find('p')
                                        .text('Device connected.');
                                      console.log('connected');
      break;
      case 2:
        $(this.GUIElements.deviceStatus).removeClass('connected')
                                        .addClass('transmitting')
                                        .find('p')
                                        .text('Device transmitting.');
        if(this.timers.timerSinceConnected == 0)
          this.timers.timerSinceConnected += 1;
      break;
    }
    
  };
GUI.prototype.sendControlCommand = function(isReturn, returnCode) {
    this.changeStatus(2);
    var message = "";
    var enableTraction = false;
    var enableSteering = false;
    var tractionSign = "";
    var steeringSign = "";
    var tractionValue = isReturn ? 0 : this.tractionValue;
    var tractionIncrementValue = this.tractionIncrementValue;
    var steeringValue = isReturn ? 0 : this.steeringValue;
    //console.log(this.pressedKeys);
    if(isReturn == true)
    {
      if(returnCode == 87 || returnCode == 83)
        enableTraction = true;
      if(returnCode == 65 || returnCode == 68)
        enableSteering = true;
    }
    else 
    {

      for(var i = 0; i < this.pressedKeys.length; i++)
      {
        var item = this.pressedKeys[i];
        if(item == 87 || item == 83) 
          enableTraction = true;
        if(item == 65 || item == 68)
          enableSteering = true;
        if(item == 83)
          tractionSign = "-";
        if(item == 65)
          steeringSign = "-";
        if(item == 221)
          tractionValue = (tractionValue + tractionIncrementValue > 250) ? 250 : tractionValue + tractionIncrementValue;
        if(item == 219)
          tractionValue = (tractionValue - tractionIncrementValue < 50) ? 50 : tractionValue - tractionIncrementValue;  
      }
    }
    tractionSign = "";

    if(enableTraction || enableSteering)
    {

      if(enableTraction)
      {
        message = "t " + tractionSign + tractionValue;
      }
      if(enableSteering)
      {
        if(message.length > 0)
          message += ", ";
        message += "steer " + steeringSign + steeringValue;
      }
      message += ";";
    }
    console.log(message);
    if(message.length)
      this.socket.send(message);
    this.updateConsole(message);
    if(isReturn != true)
    {
      this.tractionValue = tractionValue;
      this.steeringValue = steeringValue;
    }
  };
GUI.prototype.updateData = function(message) {
    var decoded_message;
    var type = "string";
    try{
      decoded_message = JSON.parse(message.data);
      type = "JSON";
    } catch(e) {
      console.log("message is not JSON");
    }
    if(type == "JSON") {
      this.changeStatus(2);
      if(decoded_message.type == "camera")
        this.updateCamera(decoded_message.data);
    }
    else
      this.updateConsole(message.data);
  };
GUI.prototype.updateCamera = function(data) {
  $(this.GUIElements.camera).attr('src', 'data:image/png;base64,' + data);
};
GUI.prototype.updateConsole = function(data) {
  $(this.GUIElements.debuggerConsole).append(data).append($('<br/>'));
};
GUI.prototype.updateTimers = function() {
  var GUI = this;
  setInterval(function(){ 
    if(GUI.timers.timerSinceConnected != 0)
      GUI.timers.timerSinceConnected += 1;
    if(GUI.timers.timerSinceStarted != 0)
      GUI.timers.timerSinceStarted += 1;
    console.log(GUI.GUIElements.timeSinceStarted);
    $('body').find(GUI.GUIElements.timeSinceStarted).text(GUI.secondsToTime(GUI.timers.timerSinceStarted));
    $('body').find(GUI.GUIElements.timeSinceConnection).text(GUI.secondsToTime(GUI.timers.timerSinceConnected));
  },1000);
};
GUI.prototype.secondsToTime = function(totalSec) {
  var hours = parseInt( totalSec / 3600 ) % 24;
  var minutes = parseInt( totalSec / 60 ) % 60;
  var seconds = totalSec % 60;

  var result = (hours < 10 ? "0" + hours : hours) + ":" + (minutes < 10 ? "0" + minutes : minutes) + ":" + (seconds  < 10 ? "0" + seconds : seconds);
  return result;
}
GUI.prototype.init = function() {
  var message = "init;";
  this.timers.timerSinceStarted += 1;
  this.updateConsole(message);
  this.socket.send(message);
};
GUI.prototype.stop = function() {
  var message = "s;";
  this.updateConsole(message);
  this.socket.send(message);
};
GUI.prototype.toggleAuto = function() {
  var message = "a;";
  this.updateConsole(message);
  this.socket.send(message);
};
GUI.prototype.setCommandMode = function(mode) {
  var previousMode = this.mode;
  var parent = this;
  this.mode = parseInt(mode);
  this.pressedKeys = [];
  console.log(this.mode);
  switch(this.mode) {
    case 0: this.sendPredefinedTestCommands();
    break;
    case 1:
      if(previousMode == 2)
        this.toggleAuto();
    break;
    case 2: 
      this.toggleAuto();
    break;
  }
};
GUI.prototype.sendPredefinedTestCommands = function() {
  var parent = this;
  var commandsNumber = this.predefinedTestCommands.length;
  var counter = 0;
  var interval = setInterval(function(){
      var message = parent.predefinedTestCommands[counter] + parent.commandFullStop;
      parent.socket.send(message);
      counter++;
      if(counter == commandsNumber) 
        clearInterval(interval);
  }, 500);
};
GUI.prototype.generateSteeringCommand = function(value) {
  return this.commands._steering + ' ' + value;
};
GUI.prototype.generateTractionCommand = function(value) {
  return this.commands._traction + ' ' + value;
};
GUI.prototype.generateRelayCommand = function(value) {
  return this.commands._relay + ' ' + value;
}
GUI.prototype.toggleRelay = function() {
  this.relayState = !this.relayState;
  this.socket.send(this.generateRelayCommand(this.relayState)+this.commandFullStop);
  return this.relayState;
};

$(document).ready(function(){
  var _GUI = new GUI();

  $(document).keydown(function(e){
    console.log(_GUI.mode);
    if(_GUI.mode == 1 || _GUI.mode == 2) {
      if(_GUI.pressedKeys.indexOf(e.keyCode) == -1 || 
        ((e.keyCode == 221 || e.keyCode == 219) && _GUI.mode == 2))
      {
        _GUI.pressedKeys.push(e.keyCode);
        _GUI.sendControlCommand();
        if((e.keyCode == 221 || e.keyCode == 219) || _GUI.mode == 2)
          _GUI.pressedKeys.pop();
      }
    }
  });
  $(document).keyup(function(e){
    if(_GUI.mode == 1 && _GUI.mode != 2) {
      var index = _GUI.pressedKeys.indexOf(e.keyCode);
      if(index != -1)
      {
        
        _GUI.pressedKeys.splice(index, 1);
        _GUI.sendControlCommand(true, e.keyCode);
      }
    }
  });
  $(document).on('click','#startButton', function(){
    _GUI.init();
  }).on('click', '#stopButton', function(){
    _GUI.stop();
  }).on('click', '#autoButton', function(){
    _GUI.toggleAuto();
  }).on('click', '#relayButton', function(){
    _GUI.stop();
    var relayState = _GUI.toggleRelay();
    var text = "Relay " + (relayState == true ? "ON" : "OFF");
    $('#relayButton').html(text);
  }).on('change', '#selectCommandMode', function(){
    var commandMode = $(this).val();
    _GUI.stop();
    _GUI.setCommandMode(commandMode);
  });
  //$(_GUI.GUIElements.batteryLevel).text(Math.floor(Math.random()*(100-80+1)+80));
});