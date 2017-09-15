# gulp-append-prepend

Simple Gulp plugin.

##Usage

First, install gulp-append-prepend as a development dependency:
```
npm install gulp-append-prepend --save-dev
```

Then, add it to your gulpfile.js:

```javascript
var gap = require('gulp-append-prepend');

gulp.task('myawesometask', function(){
    gulp.src('index.html')
    .pipe(gap.prependFile('header.html'))
    .pipe(gap.prependText('<!-- HEADER -->'))
    .pipe(gap.appendText('<!-- FOOTER -->'))
    .pipe(gap.appendFile('footer.html'))
    .pipe(gulp.dest('www/'));
});
```

This example works with html but you can use any type of file.

##Methods
- ``appendFile(filepath, separator)`` The filepath can be an array. The separator is optional by default is "\n".
- ``prependFile(filepath, separator)`` The filepath can be an array. The separator is optional by default is "\n".
- ``prependText(text, separator)`` The text can be an array. The separator is optional by default is "\n".
- ``prependText(text, separator)`` The text can be an array. The separator is optional by default is "\n".

##Licence
This plugin is released under the [MIT licence](https://raw.githubusercontent.com/NoczCore/gulp-append-prepend/master/LICENCE).

##Disclaimer
I am sorry for my poor English. Feel free to make changes in this README.
