/*
 * Press '1' or '2' to increment the number displayed by the TextBox.
 *
 * EXPECTED: The counter should continue to increment no matter how many times
 * '1' or '2' is pressed.
 */

let value = 0;

// eslint-disable-next-line no-undef
px
  .import({
    scene: 'px:scene.1.js',
    keys: 'px:tools.keys.js'
  })
  .then(imports => {
    const { scene, keys } = imports;

    let rect_one = scene.create({
      parent: scene.root,
      t: 'textBox',
      text: value,
      x: 50,
      y: 50,
      w: 100,
      h: 100,
      focus: true
    });

    function onKeyDown(event) {
      console.log('ONKEYDOWN KEY RECEIVED BY onKeyDown: ' + event.keyCode);
      const { ONE, TWO } = keys;

      switch (event.keyCode) {
        case ONE:
          value += 1;
          rect_one.text = value;
          // Remove the listener.
          rect_one.delListener('onKeyDown', onKeyDown.bind(null));
          rect_one.delListener('onKeyDown', onKeyDown.bind(rect_one));
          // Immediately re-add the listener.
          rect_one.on('onKeyDown', onKeyDown.bind(null));
          break;
        case TWO:
          value += 1;
          rect_one.text = value;
          // Remove the listener.
          rect_one.delListener('onKeyDown', onKeyDown.bind(null));
          rect_one.delListener('onKeyDown', onKeyDown.bind(rect_one));
          // Immediately re-add the listener.
          rect_one.on('onKeyDown', onKeyDown.bind(rect_one));
          break;
        default:
      }
    }

    rect_one.on('onKeyDown', onKeyDown.bind(null));
  });
