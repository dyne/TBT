function TBT(keyPressObject, textDisplayId, recordButtonId, replayButtonId, stopButtonId, instanceName) {
  // Methods
  this.handleKeyDown = handleKeyDown;
  this.handleKeyPress = handleKeyPress;
  this.keyDetect = keyDetect;
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
  this.keyDownCodes = [8, 9];
  this.keys = new Array;
  this.lastDate = new Date;
  this.startDate = new Date;
  this.timeout = new Object;

  // Static
  function keyDetect(event) {
    if (event.keyCode) {
      keyCode = event.keyCode;
    } else {
      keyCode = event.charCode;
    }
    return keyCode;
  }

  // Static
  function keysToText(keys) {
    outputText = new Array;
    keys.each(function(key) {
      if (key[0] > 0) {
        switch(key[0]) {
          // backspace
          case 8: // backspace
            outputText.pop();
            break;
          case 9: // tab
            outputText.push('&nbsp;&nbsp;&nbsp;&nbsp;');
            break;
          case 13: // enter
            outputText.push('<br />');
            break;
          case 32: // space
            outputText.push('&nbsp;');
            break;
          case 37: // left
            break;
          case 38: // up
            break;
          case 39: // right
            break;
          case 40: // down
            break;
          default:
            outputText.push(String.fromCharCode(key[0]));
        }
      }
    })
    return outputText.join('');
  }

  function record() {
    this.keys = new Array;
    this.startDate = new Date;
    this.lastDate = this.startDate;
    // Some browsers do not fire keypress for special keys (e.g. backspace),
    // so we need to listen for keydown, but we use keypress for characters
    this.keyPressObject.observe('keydown', this.handleKeyDown.bind(this));
    this.keyPressObject.observe('keypress', this.handleKeyPress.bind(this));
    $(this.textDisplayId).update('_');
    $(this.recordButtonId).update('Record');
    $(this.stopButtonId).update('<a href="javascript:' + instanceName + '.recordStop()">Stop</a>');
    $(this.replayButtonId).update('Play');
  }

  function recordStop() {
    this.keyPressObject.stopObserving('keydown');
    this.keyPressObject.stopObserving('keypress');
    this.recordKey(0); // (Record time between last key and pressing Stop)
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
      this.timeout = this.replayKeys.bind(this).delay(this.keys[this.currentKey][1]/1000);
      this.currentKey++;
    }
  }

  function replayStop() {
    window.clearTimeout(this.timeout);
    $(this.replayButtonId).update('<a href="javascript:' + instanceName + '.replay()">Play</a>');
    $(this.recordButtonId).update('<a href="javascript:' + instanceName + '.record()">Record</a>');
    $(this.stopButtonId).update('Stop');
  }

  function handleKeyDown(event) {
    keyCode = this.keyDetect(event);
    // Only handle special keys
    if (this.keyDownCodes.indexOf(keyCode) != -1) {
      this.recordKey(keyCode);
      $(this.textDisplayId).update(this.keysToText(this.keys) + '_');
      event.stop();
    }
  }

  function handleKeyPress(event) {
    keyCode = this.keyDetect(event);
    // Do not handle special keys. Do not, I said!
    if (this.keyDownCodes.indexOf(keyCode) == -1) {
      this.recordKey(keyCode);
      $(this.textDisplayId).update(this.keysToText(this.keys) + '_');
    }
    event.stop();
  }

  function recordKey(keyCode) {
    var currentDate = new Date;
    ms = currentDate.getTime() - this.lastDate.getTime();
    this.lastDate = currentDate;
    this.keys.push(new Array(keyCode, ms));
  }
}
