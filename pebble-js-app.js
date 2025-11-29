// Companion JS to send a large text to watch in chunks using AppMessage
var CHUNK_SIZE = 512;

function sendTextInChunks(text) {
  var total = text.length;
  var index = 0;
  var chunkIndex = 0;

  function sendNext() {
    if (index >= total) {
      // send finished flag
      var dict = {};
      dict[4] = 1; // KEY_FINISHED
      Pebble.sendAppMessage(dict, function() {
        console.log('Send finished flag');
      }, function(err) {
        console.log('Failed to send finished flag: ' + JSON.stringify(err));
      });
      return;
    }
    var part = text.substr(index, CHUNK_SIZE);
    var dict = {};
    dict[2] = part; // KEY_CHUNK
    dict[3] = chunkIndex; // KEY_CHUNK_INDEX
    // if this is the last chunk include finished flag
    if (index + CHUNK_SIZE >= total) {
      dict[4] = 1;
    }
    Pebble.sendAppMessage(dict, function() {
      index += CHUNK_SIZE;
      chunkIndex++;
      setTimeout(sendNext, 120);
    }, function(err) {
      console.log('Failed to send chunk: ' + JSON.stringify(err));
      // try again after small delay
      setTimeout(sendNext, 500);
    });
  }
  sendNext();
}

// ready
Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready');
});

Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL('config.html');
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e.response) return;
  try {
    var resp = JSON.parse(decodeURIComponent(e.response));
    if (resp && resp.text) {
      sendTextInChunks(resp.text);
    }
  } catch (err) {
    console.log('Error parsing response: ' + err);
  }
});