【NomIME】Bộ gõ Hán Nôm Phương Viên cho Windows
================

Dựa trên Rime Input Method Engine (中州韻輸入法引擎) và các công nghệ mã nguồn mở khác

Phương Viên Các — Copyleft



Giấy phép: GPLv3 (áp dụng cho mã nguồn chương trình)

Dữ liệu, từ điển và bảng gõ Hán Nôm đi kèm thuộc bản quyền của Phương Viên, **không** thuộc phạm vi giấy phép mã nguồn mở nói trên và không được phép sao chép, phân phối lại hay sử dụng cho mục đích khác nếu chưa có sự đồng ý bằng văn bản của Phương Viên.

Trang chủ dự án: https://bogo.hannom.org

Bạn có thể cần đến các bản phát hành Phương Viên cho hệ điều hành khác:

  * 【Phương Viên】(SinoIME) dùng cho macOS (64-bit)
  * 【Phương Viên】(iOS) dùng cho iPhone và iPad

Cài đặt bộ gõ
----------

Ứng dụng này hỗ trợ Windows 8.1 ~ Windows 11

Khi cài đặt lần đầu, trình cài đặt sẽ hiện hộp thoại "Tùy chọn cài đặt".

Để đăng ký 【NomIME】 vào bố cục bàn phím Trung văn phồn thể (Đài Loan), hãy chọn "Trung văn (Đài Loan)" trong mục "Ngôn ngữ nhập liệu", rồi bấm nút "Cài đặt".

Sau khi cài đặt xong, vẫn có thể mở "Tùy chọn cài đặt" từ Start Menu để thay đổi ngôn ngữ nhập liệu.

Sử dụng bộ gõ
----------

Chọn biểu tượng chữ 【中】trong menu chỉ báo bộ gõ để bắt đầu gõ chữ bằng Phương Viên (NomIME).

Có thể dùng phím tắt <kbd>Ctrl+`</kbd> hoặc <kbd>F4</kbd> để mở menu bảng gõ và chuyển đổi phương thức nhập.

Tùy chỉnh bộ gõ
----------

Truy cập công cụ cài đặt và các vị trí thường dùng qua Start Menu » Bộ gõ Phương Viên (NomIME).

Từ điển người dùng và tệp cấu hình nằm tại `%AppData%\NomIME`, có thể mở qua mục "Thư mục người dùng" trong menu. Người dùng nâng cao thường cần đến khi tùy chỉnh sâu bộ gõ Rime.

Sau khi sửa từ điển hoặc tệp cấu hình, cần "Triển khai lại" (Redeploy) để thay đổi có hiệu lực.

Cách tùy chỉnh Rime, vui lòng tham khảo Wiki [《Hướng dẫn tùy chỉnh》](https://github.com/rime/home/wiki/CustomizationGuide). 

Lời cảm ơn
----

### Thiết kế bảng gõ:

  * Bộ 【朙月拼音】(Mingyue Pinyin) và từ điển 【八股文】
    - Một phần dữ liệu lấy từ các dự án mã nguồn mở CC-CEDICT, Android 拼音, 新酷音, opencc, v.v.
    - Người bảo trì: 佛振, 瑾昀
  * 【注音／地球拼音】
    - Người bảo trì: 佛振, 瑾昀
  * 【倉頡五代】(Thương Hiệt ngũ đại)
    - Người phát minh: ông 朱邦復 (Chu Bang Phục)
    - Bảng mã lấy từ www.chinesecj.com
    - Tác giả bảng mã cấu từ: 惜緣

  Các bảng gõ 【五笔】【粵拼】【上海／蘇州吳語】【中古漢語拼音】【國際音標】và nhiều bảng gõ khác
  không còn được đóng gói sẵn trong bộ cài. Có thể tải và cài đặt qua <https://github.com/rime/plum>.


### Ứng dụng này sử dụng các phần mềm mã nguồn mở sau:

  * [Boost C++ Libraries](http://www.boost.org/) (Boost Software License)
  * [curl](https://curl.haxx.se/) (MIT/X derivate license)
  * [google-glog](https://github.com/google/glog) (BSD 3-Clause License)
  * [Google Test](https://github.com/google/googletest) (BSD 3-Clause License)
  * [LevelDB](https://github.com/google/leveldb) (BSD 3-Clause License)
  * [librime](https://github.com/rime/librime) (BSD 3-Clause License)
  * [marisa-trie](https://github.com/s-yata/marisa-trie) (BSD 2-Clause License, LGPL 2.1)
  * [OpenCC / 開放中文轉換](https://github.com/BYVoid/OpenCC) (Apache License 2.0)
  * [plum](https://github.com/rime/plum) (GNU Lesser General Public License v3.0)
  * [WinSparkle](https://github.com/vslavik/winsparkle) (MIT License)
  * [yaml-cpp](https://github.com/jbeder/yaml-cpp) (MIT License)
  * [7-Zip](https://www.7-zip.org) (GNU LGPLv2.1+ with unRAR restriction)

Vấn đề, phản hồi và Liên hệ
----------

Nếu phát hiện lỗi (bug) trong chương trình, 

các vấn đề về tính năng, cách dùng và cấu hình của bộ gõ hoặc

để trao đổi kỹ thuật, mời ghé thăm [Phương Viên Các](https://bogo.hannom.org). Bạn cũng có thể gửi email cho nhóm phát triển Phương Viên tại <support@hannom.org>

Trân thành cảm ơn!
