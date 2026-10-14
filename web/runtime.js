(function () {
    const codeToKey = {
        KeyA: 0, KeyB: 1, KeyC: 2, KeyD: 3, KeyE: 4, KeyF: 5, KeyG: 6, KeyH: 7, KeyI: 8,
        KeyJ: 9, KeyK: 10, KeyL: 11, KeyM: 12, KeyN: 13, KeyO: 14, KeyP: 15, KeyQ: 16,
        KeyR: 17, KeyS: 18, KeyT: 19, KeyU: 20, KeyV: 21, KeyW: 22, KeyX: 23, KeyY: 24, KeyZ: 25,
        Digit0: 26, Digit1: 27, Digit2: 28, Digit3: 29, Digit4: 30,
        Digit5: 31, Digit6: 32, Digit7: 33, Digit8: 34, Digit9: 35,
        Escape: 36,
        ControlLeft: 37, ShiftLeft: 38, AltLeft: 39, MetaLeft: 40,
        ControlRight: 41, ShiftRight: 42, AltRight: 43, MetaRight: 44,
        ContextMenu: 45,
        BracketLeft: 46, BracketRight: 47, Semicolon: 48, Comma: 49, Period: 50,
        Quote: 51, Slash: 52, Backslash: 53, Backquote: 54, Equal: 55, Minus: 56,
        Space: 57, Enter: 58, NumpadEnter: 58, Backspace: 59, Tab: 60,
        PageUp: 61, PageDown: 62, End: 63, Home: 64, Insert: 65, Delete: 66,
        NumpadAdd: 67, NumpadSubtract: 68, NumpadMultiply: 69, NumpadDivide: 70,
        ArrowLeft: 71, ArrowRight: 72, ArrowUp: 73, ArrowDown: 74,
        Numpad0: 75, Numpad1: 76, Numpad2: 77, Numpad3: 78, Numpad4: 79,
        Numpad5: 80, Numpad6: 81, Numpad7: 82, Numpad8: 83, Numpad9: 84,
        F1: 85, F2: 86, F3: 87, F4: 88, F5: 89, F6: 90, F7: 91, F8: 92, F9: 93,
        F10: 94, F11: 95, F12: 96, F13: 97, F14: 98, F15: 99,
        Pause: 100,
    };

    function attachSimpleNES(Module, canvas) {
        const ctx = canvas.getContext("2d", { alpha: false });
        let image = null;

        Module.nesPresent = function (ptr, w, h) {
            if (!image || image.width !== w || image.height !== h) {
                image = ctx.createImageData(w, h);
            }
            const heap = Module.HEAPU8;
            image.data.set(heap.subarray(ptr, ptr + w * h * 4));
            ctx.putImageData(image, 0, 0);
        };

        function onKey(event, down) {
            const key = codeToKey[event.code];
            if (key === undefined) {
                return;
            }
            event.preventDefault();
            Module._nes_set_key(key, down ? 1 : 0);
        }

        canvas.addEventListener("keydown", (event) => onKey(event, true));
        canvas.addEventListener("keyup", (event) => onKey(event, false));
        window.addEventListener("blur", () => Module._nes_release_keys());

        return {
            loadRom(bytes) {
                const ptr = Module._malloc(bytes.length);
                Module.HEAPU8.set(bytes, ptr);
                const rc = Module._nes_load_rom(ptr, bytes.length);
                Module._free(ptr);
                canvas.focus();
                return rc;
            },
        };
    }

    window.attachSimpleNES = attachSimpleNES;
})();
