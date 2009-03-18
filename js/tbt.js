function TBT(keyPressObject, textDisplayId, recordButtonId, replayButtonId, stopButtonId, instanceName) {
  // Methods
  this.handleKeyPress = handleKeyPress;
  this.keysToText = keysToText;
  this.record = record;
  this.recordKey = recordKey;
  this.recordStop = recordStop;
  this.replay = replay;
  this.replayKeys = replayKeys;
  this.replayStop = replayStop;

  // Contructor properties
  this.instanceName = instanceName;
  this.keyPressObject = keyPressObject;
  this.replayButtonId = replayButtonId;
  this.stopButtonId = stopButtonId;
  this.textDisplayId = textDisplayId;
  this.recordButtonId = recordButtonId;

  // Properties
  this.currentKey = 0;
  this.keys = new Array;
  this.lastDate = new Date;
  this.startDate = new Date;
  this.timeout = new Object;

  // Static
  function keysToText(keys) {
    outputText = new Array;
    keys.each(function(key) {
      if (key[0] > 0) {
        switch(key[0]) {
          // backspace
          case 8:
            outputText.pop();
            break;
          case 13:
            outputText.push('<br />');
            break;
        }
      } else if (key[1] > 0) {
        outputText.push(String.fromCharCode(key[1]));
      }
    })
    return outputText.join('');
  }

  function record() {
    this.keys = new Array;
    this.startDate = new Date;
    this.lastDate = this.startDate;
    this.keyPressObject.observe('keypress', this.handleKeyPress.bind(this));
    $(this.textDisplayId).update('_');
    $(this.recordButtonId).update('Record');
    $(this.stopButtonId).update('<a href="javascript:' + instanceName + '.recordStop()">Stop</a>');
    $(this.replayButtonId).update('Play');
  }

  function recordStop() {
    this.keyPressObject.stopObserving('keypress');
    this.recordKey(0, 0); // (Record time between last key and pressing Stop)
    $(this.recordButtonId).update('<a href="javascript:' + instanceName + '.record()">Record</a>');
    $(this.replayButtonId).update('<a href="javascript:' + instanceName + '.replay()">Play</a>');
    $(this.stopButtonId).update('Stop');
  }

  function replay() {
    this.currentKey = 0;
    $(this.stopButtonId).update('<a href="javascript:' + instanceName + '.replayStop()">Stop</a>');
    $(this.recordButtonId).update('Record');
    $(this.replayButtonId).update('Play');
    $(this.textDisplayId).update('_');
    this.replayKeys();
  }

  function replayKeys() {
    if (this.currentKey > 0) {
      $(this.textDisplayId).update(this.keysToText(this.keys.slice(0, this.currentKey)) + '_');
    }
    if (this.currentKey == this.keys.length) {
      this.replayStop();
      return;
    }
    if (this.currentKey + 1 <= this.keys.length) {
      this.timeout = this.replayKeys.bind(this).delay(this.keys[this.currentKey][2]/1000);
      this.currentKey++;
    }
  }

  function replayStop() {
    window.clearTimeout(this.timeout);
    $(this.replayButtonId).update('<a href="javascript:' + instanceName + '.replay()">Play</a>');
    $(this.recordButtonId).update('<a href="javascript:' + instanceName + '.record()">Record</a>');
    $(this.stopButtonId).update('Stop');
  }

  function handleKeyPress(event) {
    this.recordKey(event.keyCode, event.charCode);
    $(this.textDisplayId).update(this.keysToText(this.keys) + '_');
  }

  function recordKey(keyCode, charCode) {
    var currentDate = new Date;
    ms = currentDate.getTime() - this.lastDate.getTime();
    this.lastDate = currentDate;
    this.keys.push(new Array(keyCode, charCode, ms));
  }
}
