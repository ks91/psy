"""Black-box editor tests: real terminal input, file contents and cursor positions."""
import os
import fcntl
import signal
import struct
import termios
from pathlib import Path
import pty
import re
import select
import subprocess
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parents[1]
BINARY = Path(os.environ.get('PSY_BINARY', ROOT / 'psy')).resolve()
SAVE = b'\x18\x17'
EXIT = b'\x18\x03'
END = b'\x0c\x16'
POINT = b'\x18\x10'


class Editor:
    def __init__(self, data=b'', options=(), config=None, filename='sample.txt', size=(24, 80)):
        self.temp = tempfile.TemporaryDirectory(prefix='psy-test-', dir='/tmp')
        self.directory = Path(self.temp.name)
        self.file = self.directory / filename
        self.file.write_bytes(data)
        (self.directory / 'custom.psy').write_bytes(
            config if config is not None else (ROOT / 'tests/custom.psy').read_bytes())
        self.master, slave = pty.openpty()
        fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack('HHHH', *size, 0, 0))
        self.output = bytearray()
        self.process = subprocess.Popen(
            [str(BINARY), *options, str(self.file)], stdin=slave, stdout=slave,
            stderr=slave, cwd=self.directory, start_new_session=True,
            env=dict(os.environ, TERM='xterm-256color', LC_ALL='en_US.UTF-8',
                     PSYHEAD=str(self.directory) + '/'))
        os.close(slave)
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            self.drain(.05)
            if (self.output.count(b'Psy:') >= 2 and
                    self.output.endswith(b'\x1b[1;1H')):
                break
            if b'Enlarge terminal' in self.output:
                break
            if self.process.poll() is not None:
                break
        else:
            self.close()
            raise AssertionError('Editor did not finish opening the file')

    def drain(self, seconds=.1):
        deadline = time.monotonic() + seconds
        while time.monotonic() < deadline:
            if not select.select([self.master], [], [], max(0, deadline-time.monotonic()))[0]:
                break
            try:
                data = os.read(self.master, 65536)
            except OSError:
                break
            if not data:
                break
            self.output.extend(data)

    def send(self, keys):
        if isinstance(keys, str):
            keys = keys.encode('utf-8')
        if self.process.poll() is not None:
            raise AssertionError(bytes(self.output[-6000:]))
        os.write(self.master, keys)
        self.drain()
        if (b'ERROR: AddressSanitizer' in self.output or
                b'runtime error:' in self.output or
                (keys != EXIT and self.process.poll() is not None)):
            raise AssertionError(bytes(self.output[-6000:]))

    def finish(self):
        self.send(SAVE)
        self.send(EXIT)
        self.process.wait(timeout=5)
        if self.process.returncode:
            raise AssertionError((self.process.returncode, self.output[-1000:]))
        if b'ERROR: AddressSanitizer' in self.output or b'runtime error:' in self.output:
            raise AssertionError(bytes(self.output[-6000:]))
        return self.file.read_bytes()

    def resize(self, rows, columns):
        self.output.clear()
        fcntl.ioctl(self.master, termios.TIOCSWINSZ,
                    struct.pack('HHHH', rows, columns, 0, 0))
        os.kill(self.process.pid, signal.SIGWINCH)
        self.drain(.2)

    def close(self):
        if self.process.poll() is None:
            self.process.kill()
            self.process.wait()
        os.close(self.master)
        self.temp.cleanup()


class EditorTests(unittest.TestCase):
    def editor(self, *args, **kwargs):
        result = Editor(*args, **kwargs)
        self.addCleanup(result.close)
        return result

    def test_ascii_edit_and_backup(self):
        e = self.editor(b'old\n')
        e.send('new ')
        self.assertEqual(e.finish(), b'new old\n')
        self.assertEqual(e.file.with_suffix('.bak').read_bytes(), b'old\n')

    def test_capacity_display_with_legacy_configuration(self):
        config = (ROOT / 'tests/custom.psy').read_text().replace(
            'msg.info.size = "size";',
            'msg.info.size = "テキストサイズ = %ld; バッファの残り = (%ld 文字，%ld 行) です。";')
        e = self.editor('A日😀\n'.encode(), config=config.encode('cp932'))
        e.output.clear()
        e.send(b'\x18\x0c\x10')
        self.assertIn('自動拡張 | 4文字 | 確保済み空き'.encode(), e.output)
        self.assertNotIn('バッファの残り'.encode(), e.output)
        self.assertEqual(e.finish(), 'A日😀\n'.encode())

    def test_capacity_display_after_growth(self):
        data = b'abcd\n' * 15000
        e = self.editor(data)
        e.output.clear()
        e.send(b'\x18\x0c\x10')
        self.assertIn('自動拡張 | 75000文字 | 確保済み空き'.encode(), e.output)
        self.assertEqual(e.finish(), data)

    def test_capacity_display_custom_template(self):
        config = (ROOT / 'tests/custom.psy').read_bytes() + (
            b'msg.info.capacity = "AUTO %ld chars; allocated free %ld chars/%ld lines";\n')
        e = self.editor(b'abc\n', config=config)
        e.output.clear()
        e.send(b'\x18\x0c\x10')
        self.assertRegex(e.output, rb'AUTO 4 chars; allocated free \d+ chars/\d+ lines')
        self.assertEqual(e.finish(), b'abc\n')

    def test_terminal_size_at_startup(self):
        e = self.editor(b'A' * 95 + b'VISIBLE\n', size=(35, 120))
        self.assertIn(b'VISIBLE', e.output)
        self.assertIn(b'\x1b[33;1H', e.output)  # mode line
        self.assertIn(b'\x1b[34;1H', e.output)  # minibuffer
        self.assertEqual(e.finish(), b'A' * 95 + b'VISIBLE\n')

    def test_resize_while_idle_then_edit(self):
        e = self.editor(b'A' * 95 + b'VISIBLE\n')
        e.resize(40, 120)
        self.assertIn(b'VISIBLE', e.output)  # redraw without pressing a key
        self.assertIn(b'\x1b[38;1H', e.output)
        e.send('新')
        e.resize(12, 40)
        self.assertIn(b'\x1b[10;1H', e.output)
        self.assertEqual(e.finish(), '新'.encode() + b'A' * 95 + b'VISIBLE\n')

    def test_resize_minibuffer_preserves_input_and_prompt(self):
        e = self.editor('前 日本 後\n'.encode())
        e.send(b'\x0c\x03')
        e.send('日本/')
        e.resize(30, 100)
        self.assertIn(b'\x1b[30;1H', e.output)
        self.assertIn(b'search', e.output)
        e.send('東京\r')
        e.send(b'\x0c\x17')
        self.assertEqual(e.finish(), '前 東京 後\n'.encode())

    def test_resize_split_windows_and_tiny_terminal(self):
        e = self.editor(b'original\n')
        e.send(b'\x18\x0c\x0f')  # split window
        e.resize(32, 100)
        self.assertEqual(e.output.count(b'Psy:'), 2)
        e.resize(2, 5)
        self.assertIsNone(e.process.poll())
        e.resize(24, 80)
        self.assertEqual(e.output.count(b'Psy:'), 2)
        e.send('OK ')
        self.assertEqual(e.finish(), b'OK original\n')

    def test_resize_between_utf8_bytes(self):
        e = self.editor()
        e.send(b'\xf0\x9f')
        e.resize(30, 100)
        e.send(b'\x98\x80')
        self.assertEqual(e.finish(), '😀'.encode())

    def test_resize_preserves_repeat_argument(self):
        e = self.editor(b'abcdef\n')
        e.send(b'\x15')
        e.resize(30, 100)
        e.send(b'\x06')
        e.send(POINT)
        self.assertIn(b'POINT 1 3', e.output)
        self.assertEqual(e.finish(), b'abcdef\n')

    def test_resize_during_save_confirmation(self):
        e = self.editor(b'old\n')
        e.send('new ')
        e.send(EXIT)
        e.resize(30, 100)
        self.assertIn(b'save', e.output)
        os.write(e.master, b'y')
        e.drain(.2)
        self.assertEqual(e.process.wait(timeout=5), 0)
        self.assertEqual(e.file.read_bytes(), b'new old\n')

    def test_resize_from_tiny_startup(self):
        e = self.editor(b'old\n', size=(3, 20))
        e.resize(24, 80)
        self.assertIn(b'Psy:', e.output)
        e.send('new ')
        self.assertEqual(e.finish(), b'new old\n')

    def test_resize_preserves_scrolled_cursor(self):
        data = (('日本語' * 60 + '\n') * 80 + '末尾' * 60).encode()
        e = self.editor(data)
        e.send(END)
        for rows, columns in [(8, 30), (45, 140), (4, 20)]:
            e.resize(rows, columns)
            positions = re.findall(rb'\x1b\[(\d+);(\d+)H', e.output)
            self.assertTrue(positions)
            self.assertTrue(all(1 <= int(y) <= rows and 1 <= int(x) <= columns
                                for y, x in positions))
        e.send('😀')
        self.assertEqual(e.finish(), data + '😀'.encode())

    def test_over_one_million_characters(self):
        data = (b'abcdefghij' * 10 + b'\n') * 12000
        e = self.editor(data)
        e.send(END)
        e.send('TAIL')
        self.assertEqual(e.finish(), data + b'TAIL')

    def test_over_32767_lines(self):
        data = b'x\n' * 40000
        e = self.editor(data)
        e.send(END)
        e.send(POINT)
        self.assertIn(b'POINT 40001 1', e.output)
        e.send('END')
        self.assertEqual(e.finish(), data + b'END')

    def test_long_line_and_backspace(self):
        data = b'a' * 100000
        e = self.editor(data)
        e.send(b'\x05')
        e.send(POINT)
        self.assertIn(b'POINT 1 100001', e.output)
        e.send(b'\x08Z')
        self.assertEqual(e.finish(), data[:-1] + b'Z')

    def test_utf8_roundtrip_and_terminal_output(self):
        data = '日本語 café Ελληνικά 😀 𠮷\n'.encode()
        e = self.editor(data)
        self.assertIn('日本語 café Ελληνικά 😀 𠮷'.encode(), e.output)
        self.assertEqual(e.finish(), data)

    def test_utf8_input_cursor_delete(self):
        e = self.editor()
        e.send('Aあé😀')
        e.send(POINT)
        self.assertIn(b'POINT 1 7', e.output)
        e.send(b'\x02\x04')  # back one character, delete emoji
        e.send(b'\x08')      # delete accent, not a UTF-8 byte
        self.assertEqual(e.finish(), 'Aあ'.encode())

    def test_combining_mark_width(self):
        e = self.editor()
        e.send('e\u0301X')
        e.send(POINT)
        self.assertIn(b'POINT 1 3', e.output)
        self.assertEqual(e.finish(), 'e\u0301X'.encode())

    def test_utf8_search_and_replace(self):
        e = self.editor('前 日本 後\n'.encode())
        e.send(b'\x0c\x03')
        e.send('日本/東京\r')
        e.send(b'\x0c\x17')  # search and replace
        self.assertEqual(e.finish(), '前 東京 後\n'.encode())

    def test_long_regex_match(self):
        e = self.editor(b'a' * 10000 + b'\n')
        e.send(b'\x0c\x03')
        e.send('a*/X\r')
        e.send(b'\x0c\x17')
        self.assertEqual(e.finish(), b'X\n')

    def test_long_regex_transliteration(self):
        e = self.editor(b'a' * 10000 + b'\n')
        e.send(b'\x0c\x03')
        e.send('[a]*/[b]*\r')
        e.send(b'\x0c\x17')
        self.assertEqual(e.finish(), b'b' * 10000 + b'\n')

    def test_auto_prefers_valid_utf8(self):
        # These bytes are also valid CP932, but must be interpreted as UTF-8.
        data = 'é'.encode()
        e = self.editor(data)
        e.send(b'\x06')
        e.send(POINT)
        self.assertIn(b'POINT 1 2', e.output)
        self.assertEqual(e.finish(), data)

    def test_invalid_import_leaves_buffer_intact(self):
        e = self.editor(b'keep\n')
        old = e.directory / 'bad.txt'
        old.write_bytes(b'prefix\x81')
        e.send(b'\x18\x06')
        e.send(str(old) + '\r')
        self.assertEqual(e.finish(), b'keep\n')

    def test_unicode_copy_paste_and_pool_reuse(self):
        text = '日本語😀é' * 200
        e = self.editor((text + '\n').encode())
        e.send(b'\x18\x01')  # begin block
        e.send(b'\x05')      # end of line
        e.send(b'\x18\x05')  # end block
        e.send(END)
        e.send(b'\x18\x19')  # copy block to kill buffer
        e.send(b'\x19')      # yank after original newline
        self.assertEqual(e.finish(), (text + '\n' + text).encode())

    def test_utf8_filename_with_spaces(self):
        e = self.editor(b'old\n', filename='日本 語.txt')
        e.send('新')
        self.assertEqual(e.finish(), '新old\n'.encode())
        self.assertEqual(e.file.with_suffix('.bak').read_bytes(), b'old\n')

    def test_shift_jis_explicit(self):
        e = self.editor('日本語\n'.encode('cp932'), options=(
            '--read-encoding=shift-jis', '--write-encoding=shift-jis'))
        self.assertIn('日本語'.encode(), e.output)
        e.send('新')
        self.assertEqual(e.finish(), '新日本語\n'.encode('cp932'))

    def test_unrepresentable_shift_jis_does_not_replace_file(self):
        data = '日本語\n'.encode('cp932')
        e = self.editor(data, options=('--write-encoding=shift-jis',))
        e.file.with_suffix('.bak').write_bytes(b'previous backup')
        e.send('😀')
        e.send(SAVE)
        self.assertEqual(e.file.read_bytes(), data)
        self.assertEqual(e.file.with_suffix('.bak').read_bytes(), b'previous backup')
        self.assertEqual(list(e.directory.glob('*.psy.*')), [])
        self.assertIsNone(e.process.poll())

    def test_invalid_utf8_does_not_partially_load_and_overwrite(self):
        for data in [b'prefix\xffsuffix', b'prefix\xe3\x81', b'prefix\x00suffix']:
            with self.subTest(data=data):
                e = self.editor(data, options=('--read-encoding=utf-8',))
                e.send(SAVE)
                self.assertEqual(e.file.read_bytes(), data)

    def test_auto_shift_jis_saves_utf8(self):
        original = '日本語と半角ｶﾅ\n'.encode('cp932')
        e = self.editor(original)
        self.assertIn('日本語と半角ｶﾅ'.encode(), e.output)
        e.send(END)
        e.send('😀𠮷🧪')
        self.assertEqual(e.finish(), '日本語と半角ｶﾅ\n😀𠮷🧪'.encode())
        self.assertEqual(e.file.with_suffix('.bak').read_bytes(), original)

    def test_insert_shift_jis_file_into_utf8_buffer(self):
        e = self.editor('新しい文章\n'.encode())
        old = e.directory / 'old.txt'
        old.write_bytes('古い文章\n'.encode('cp932'))
        e.send(END)
        e.send(b'\x18\x06')
        e.send(str(old) + '\r')
        self.assertEqual(e.finish(), '新しい文章\n古い文章\n'.encode())

    def test_invalid_in_both_encodings_does_not_overwrite(self):
        for data in [b'prefix\x81\x20suffix', b'prefix\x81', b'prefix\x00suffix']:
            with self.subTest(data=data):
                e = self.editor(data)
                e.send(SAVE)
                self.assertEqual(e.file.read_bytes(), data)

    def test_newlines(self):
        for data in [b'a\r\nb\r\n', b'a\rb\r']:
            with self.subTest(data=data):
                e = self.editor(data)
                self.assertEqual(e.finish(), b'a\nb\n')

    def test_small_initial_pool_grows(self):
        config = (ROOT / 'tests/custom.psy').read_bytes()
        config = config.replace(b'65535;', b'64;').replace(b'4096;', b'8;')
        data = ('日本語\n' * 1000).encode()
        e = self.editor(data, config=config)
        e.send(END)
        e.send('終')
        self.assertEqual(e.finish(), data + '終'.encode())

    def test_shift_jis_configuration(self):
        config = (ROOT / 'tests/custom.psy').read_text().replace('"INS"', '"挿入"')
        e = self.editor(config=config.encode('cp932'))
        self.assertIn('挿入'.encode(), e.output)
        self.assertEqual(e.finish(), b'')


if __name__ == '__main__':
    unittest.main()
