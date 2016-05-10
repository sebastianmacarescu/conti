var GUI = function(){
  var parent = this;
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
    parent.processCommand(message);
  }; 
  
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
GUI.prototype.sendCommand = function(isReturn, returnCode) {
    this.changeStatus(2);
    var message = "";
    var enableTraction = false;
    var enableSteering = false;
    var tractionSign = "";
    var steeringSign = "";
    var tractionValue = isReturn ? 0 : this.tractionValue;
    var tractionIncrementValue = this.tractionIncrementValue;
    var steeringValue = isReturn ? 0 : this.steeringValue;

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
        if(item == 16)
          tractionValue = (tractionValue + tractionIncrementValue > 255) ? 255 : tractionValue + tractionIncrementValue;
        if(item == 17)
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

    if(message.length)
      this.socket.send(message);
    this.processCommand({data:message});
    if(isReturn != true)
    {
      this.tractionValue = tractionValue;
      this.steeringValue = steeringValue;
    }
  };
GUI.prototype.processCommand = function(message) {
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
        $(this.GUIElements.camera).attr('src', 'data:image/png;base64,' + decoded_message.data);
    }
    else
      $(this.GUIElements.debuggerConsole).append(message.data).append($('<br/>'));
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
  this.processCommand({data:message});
  this.socket.send(message);
};
GUI.prototype.stop = function() {
  var message = "stop;";
  this.processCommand({data:message});
  this.socket.send(message);
};

$(document).ready(function(){
  var _GUI = new GUI();

  $(document).keydown(function(e){
    if(_GUI.pressedKeys.indexOf(e.keyCode) == -1)
    {
      _GUI.pressedKeys.push(e.keyCode);
      _GUI.sendCommand();
    }
  });
  $(document).keyup(function(e){
    var index = _GUI.pressedKeys.indexOf(e.keyCode);
    if(index != -1)
    {
      
      _GUI.pressedKeys.splice(index, 1);
      _GUI.sendCommand(true, e.keyCode);
    }
  });
  $(document).on('click','#startButton', function(){
    _GUI.init();
  }).on('click', '#stopButton', function(){
    _GUI.stop();
  });
  //$(_GUI.GUIElements.batteryLevel).text(Math.floor(Math.random()*(100-80+1)+80));
});




