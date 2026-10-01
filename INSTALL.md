# Cách build Rime với NomIME

## Chuẩn bị

  - Cài **Visual Studio 2017** cho mục *Desktop development in C++*
    kèm các thành phần *ATL*, *MFC* và *Windows XP support*.
    Visual Studio 2015 trở lên cũng có thể dùng được nếu cấu hình thêm.

  - Cài các công cụ phát triển: `git`, `cmake`， `clang-format(>=17.0.6)`

  - Tải thư viện bên thứ ba: `boost(>=1.60.0)`

Tùy chọn thêm:

  - cài `bash` qua *Git for Windows*, để cài dữ liệu bằng `plum`;
  - cài `python` để build từ điển OpenCC;
  - cài [NSIS](http://nsis.sourceforge.net/Download) để tạo trình cài đặt.

## Lấy mã nguồn

Đảm bảo tất cả git submodule đều được checkout đệ quy.

```batch
git clone --recursive https://github.com/VienChi/nomime.git
```

## Build và cài đặt NomIME

Vào thư mục mã nguồn `nomime`.

### Thiết lập môi trường build

Chỉnh các thiết lập môi trường build trong `env.bat`.
Bạn có thể tạo file này bằng cách sao chép `env.bat.template` trong thư mục mã nguồn.

Đảm bảo `BOOST_ROOT` trỏ đúng đến đường dẫn `X:\path\to\boost_<version>` đã tồn tại.

Nếu dùng phiên bản Visual Studio hoặc platform toolset khác, hãy bỏ comment
các dòng tương ứng để thiết lập biến phù hợp.

Cách khác: mở cửa sổ *Developer Command Prompt* và đặt biến môi trường
trực tiếp trong console, trước khi chạy `build.bat`:

```batch
set BOOST_ROOT=X:\path\to\boost_N_NN_N
```

### Build

```batch
cd nomime
build.bat all
```

Xong.

Trình cài đặt sẽ được tạo ra trong thư mục `output\archives`.

### Cách khác: dùng bản dựng sẵn (prebuilt) của Rime

Nếu bạn đã có sẵn bản build của librime, chỉ cần
sao chép các file `.dll` / `.lib` vào lần lượt thư mục
`nomime\output` / `nomime\lib`, sau đó build NomIME mà không cần tham số dòng lệnh `all`.

```batch
build.bat boost data opencc
build.bat nomime
```

### Cài đặt và dùng thử

```batch
cd output
install.bat
```

### Tùy chọn: dùng thử công cụ dòng lệnh của Rime

`librime` đi kèm một ứng dụng REPL dùng để kiểm tra xem thư viện
có hoạt động hay không.

```batch
cd librime
copy /Y build\lib\Release\rime.dll build\bin
cd build\bin
echo zhongzhouyunshurufa | Release\rime_api_console.exe > output.txt
```

Thay vì chuyển hướng output ra file, bạn có thể đặt code page phù hợp
(`chcp 65001`) và font trong console để dùng REPL một cách tương tác.
