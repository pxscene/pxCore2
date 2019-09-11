/*
* If not stated otherwise in this file or this component's license file the
* following copyright and licenses apply:
*
* Copyright 2018 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

window.onload = function() {
    var aBox = document.getElementById("firstBox");
    var bBox = document.getElementById("secondBox");
    var cBox = document.getElementById("thirdBox");

    var currentObj = aBox;
    var currentPos = 0;
    //make first box defaultly highlighted
    currentObj.classList.add("focus");
    components = [aBox, bBox, cBox];
    window.onkeydown = function(elt) {
        var keyCode = elt.keyCode || elt.which;

        if (keyCode === 37) {
            removeFocus();
            if (currentPos > 0) {
                currentPos--;
            } else {
                currentPos = components.length - 1;
            }
            currentObj = components[currentPos];
            addFocus();
        } else if (keyCode === 39) {
            removeFocus();
            if (currentPos < components.length - 1) {
                currentPos++;
            } else {
                currentPos = 0;
            }
            currentObj = components[currentPos];
            addFocus();

        } else if ((keyCode == 13) || (keyCode == 32)) {
            switch (currentPos) {
                case 0:
                    window.location.href = "VIDEOTAG/index.html";
                    break;
                case 1:
                    window.location.href = "UVE/index.html";
                    break;
                case 2:
                    window.location.href = "http://www.sparkui.org";
            }
        }
    }

    addFocus = function() {
        if (currentObj) {
            currentObj.classList.add("focus");
        } else {
            currentObj.focus();
        }
    };

    removeFocus = function() {
        if (currentObj) {
            currentObj.classList.remove("focus");
        } else {
            currentObj.blur();
        }
    }
}
