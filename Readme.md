#        CÓ 3 PHẦN CHÍNH
1/ Hướng dẫn cài đặt
2/ Hướng dẫn thay đổi ảnh nền

#####################################################
# 1 Hướng Dẫn Cài Đặt Desktop (Screen Saver)
# 📋 Danh Sách Công Việc trong 6 bước
#####################################################

### Bước 1: Copy Files Desktop
Sao chép 2 file này từ xiaozhivn sang project mới:
```
desktop.h    → main/tools/desktop/
desktop.cc   → main/tools/desktop/
```

---

## 🔧 Bước 2: Sửa CMakeLists.txt

**Vị trí**: `main/CMakeLists.txt`

Thêm dòng này vào danh sách SOURCES:
```cmake
# Line ~43
SOURCES
    "application.cc"
    "assets.cc"
    ...
	"tools/desktop/icons.c"
    "tools/desktop/desktop.cc"  # ← THÊM DÒNG NÀY
    ...
```

Thêm dòng này vào danh sách INCLUDE_DIRS:
```cmake
# Line ~47
INCLUDE_DIRS
    "."
    "tools/desktop"  # ← THÊM DÒNG NÀY
    ...
```

---

## 🔗 Bước 3: Cập Nhật application.h

**Vị trí**: `main/application.h`

### 3.1 Thêm Include
```cpp
// Line đầu file, sau các include khác
#include "desktop.h"
```

### 3.2 Thêm Member Variable
```cpp
// Trong private section, khoảng line 90-100
private:
    Application();
    ~Application();
    
    // ... các variable khác ...
    
    Desktop* desktop_ = nullptr;  // ← THÊM DÒNG NÀY
    
    // ... các variable khác ...
```

---

## 🔌 Bước 4: Cập Nhật application.cc

**Vị trị**: `main/application.cc`

### 4.1 Khởi Tạo Desktop trong Start()
Tìm hàm `void Application::Start()`, khoảng cuối hàm (sau khi khởi tạo display), thêm:

```cpp
// Khoảng line 420-430, sau khởi tạo các system khác
void Application::Start() {
    // ... code khởi tạo khác ...
    
    // Initialize Desktop for black screensaver
    ESP_LOGI(TAG, "🔍 Initializing Desktop screensaver...");
    desktop_ = new Desktop(this, (LcdDisplay*)display);
    if (desktop_ != nullptr) {
        ESP_LOGI(TAG, "✓ Desktop initialized successfully");
        desktop_->Start();
        ESP_LOGI(TAG, "✓ Desktop started");
    } else {
        ESP_LOGE(TAG, "✗ Failed to create Desktop instance");
    }
    
    // ... code sau ...
}
```

### 4.2 Gọi desktop_->Update() trong MainEventLoop()
Tìm hàm `void Application::MainEventLoop()`, tìm đoạn xử lý `MAIN_EVENT_CLOCK_TICK`, sửa như sau:

```cpp
// Khoảng line 664-680
if (bits & MAIN_EVENT_CLOCK_TICK) {
    clock_ticks_++;
    auto display = Board::GetInstance().GetDisplay();
    display->UpdateStatusBar();
    
    // Update desktop (check DeviceState and show/hide screensaver)
    if (desktop_ != nullptr) {
        ESP_LOGI(TAG, "[TICK] Calling desktop_->Update() - clock_ticks=%d", clock_ticks_);
        desktop_->Update();
    } else {
        ESP_LOGW(TAG, "[TICK] desktop_ is nullptr!");
    }
    
}
```

---

## 📊 Bước 5: Kiểm Tra Compile

Build project:
```bash
idf.py build
```

**Nếu có lỗi**:
- ❌ `cannot find desktop.h` → Kiểm tra CMakeLists.txt INCLUDE_DIRS
- ❌ `undefined reference to Desktop::` → Kiểm tra CMakeLists.txt SOURCES có dòng desktop.cc không
- ❌ `Application::desktop_` undefined → Kiểm tra application.h có khai báo member variable không

---

## 🧪 Bước 6: Test Runtime

Flash và monitor:
```bash
idf.py build flash monitor
```

#####################################################
# 2 Hướng Dẫn thay đổi ảnh nền
#####################################################

#  Tạo file hex cho ảnh nền
// Tìm 1 ảnh nền ưng ý
// vào web : https://lvgl.io/tools/imageconverter
// chọn LVGL v9, RGB 565 -> Convert và tải về file .c

#  Copy mã hex vào file icons.c
// VD : file ảnh có tên matdongho.png tải về file matdongho.c
// Copy các dòng này từ matdongho.c vào file icons.c

#ifndef LV_ATTRIBUTE_IMAGE_MATDONGHO
#define LV_ATTRIBUTE_IMAGE_MATDONGHO
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMAGE_MATDONGHO uint8_t matdongho_map[] = {
  0x82, 0x10, ...
};
 
const lv_image_dsc_t matdongho = {
  .header.cf = LV_COLOR_FORMAT_RGB565,
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.w = 320,
  .header.h = 320,
  .data_size = 102400 * 2,
  .data = matdongho_map,
};

//  Thay  tên ảnh nền vào code từ dòng 94
background.data
&background


#####################################################
# 3 🎨 Hướng Dẫn Tùy Chỉnh Giao Diện Screen Saver
#####################################################

## 📝 1. Tùy Chỉnh Text (Thời Gian)

### 1.1 Thay Đổi Màu Chữ
**Vị trí**: `desktop.cc` → hàm `CreateTimeLabel()`

```cpp
// Đổi màu trắng (0xFFFFFF) thành màu khác
lv_obj_set_style_text_color(ui_time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

// Ví dụ màu khác:
// Đỏ:      0xFF0000
// Xanh:    0x00FF00
// Vàng:    0xFFFF00
// Cam:     0xFF8000
// Hồng:    0xFF1493
```

### 1.2 Thay Đổi Kích Thước Font
```cpp
// Dòng hiện tại:
lv_obj_set_style_text_font(ui_time_label, &lv_font_montserrat_14, LV_PART_MAIN);

// Các font có sẵn:
&lv_font_montserrat_10     // Nhỏ
&lv_font_montserrat_14     // Vừa (hiện tại)
&lv_font_montserrat_20     // Lớn
&lv_font_montserrat_28     // Rất lớn
&lv_font_montserrat_36     // Khổng lồ
```

### 1.3 Thay Đổi Scale (Phóng To/Thu Nhỏ)
```cpp
// Dòng hiện tại:
lv_obj_set_style_transform_scale(ui_time_label, 512, LV_PART_MAIN);

// Scale là phần trăm (256 = 100%):
256   // 100% (bình thường)
384   // 150% (phóng to 1.5x)
512   // 200% (phóng to 2x)
640   // 250% (phóng to 2.5x)
768   // 300% (phóng to 3x)
128   // 50%  (thu nhỏ)
```

### 1.4 Thay Đổi Vị Trí Text
```cpp
// Dòng hiện tại:
int32_t label_width = lv_obj_get_self_width(ui_time_label);
lv_obj_align(ui_time_label, LV_ALIGN_TOP_MID, -label_width / 2, 25);

// Tham số:
// Param 1: LV_ALIGN_TOP_MID     → Căn theo trên-giữa
// Param 2: -label_width / 2     → Offset X (âm = sang trái, dương = sang phải)
// Param 3: 25                   → Offset Y (dương = xuống, âm = lên)

// Các vị trí khác:
LV_ALIGN_CENTER               // Giữa màn hình
LV_ALIGN_TOP_MID              // Trên-giữa (hiện tại)
LV_ALIGN_BOTTOM_MID           // Dưới-giữa
LV_ALIGN_LEFT_MID             // Giữa-trái
LV_ALIGN_RIGHT_MID            // Giữa-phải

// Ví dụ: đặt ở giữa màn hình
lv_obj_align(ui_time_label, LV_ALIGN_CENTER, 0, 0);

// Ví dụ: đặt ở dưới-giữa, cách dưới 20px
lv_obj_align(ui_time_label, LV_ALIGN_BOTTOM_MID, 0, -20);
```

### 1.5 Thay Đổi Định Dạng Thời Gian
**Vị trí**: `desktop.cc` → hàm `UpdateTimeLabel()`

```cpp
// Hiện tại: HH:MM:SS
lv_label_set_text_fmt(ui_time_label, "%02d:%02d:%02d", info->tm_hour, info->tm_min, info->tm_sec);

// Chỉ HH:MM (không có giây)
lv_label_set_text_fmt(ui_time_label, "%02d:%02d", info->tm_hour, info->tm_min);

// Hiển thị ngày-tháng-năm
lv_label_set_text_fmt(ui_time_label, "%02d/%02d/%04d", info->tm_mday, info->tm_mon + 1, info->tm_year + 1900);

// Kết hợp cả hai
lv_label_set_text_fmt(ui_time_label, "%02d:%02d:%02d | %02d/%02d", info->tm_hour, info->tm_min, info->tm_sec, info->tm_mday, info->tm_mon + 1);
```

### 1.6 Thêm Padding/Khoảng Cách
```cpp
// Sau khi create label, thêm khoảng cách xung quanh text
lv_obj_set_style_pad_all(ui_time_label, 10, LV_PART_MAIN);  // Khoảng cách: 10px

// Hoặc riêng từng hướng:
lv_obj_set_style_pad_top(ui_time_label, 5, LV_PART_MAIN);     // Trên
lv_obj_set_style_pad_bottom(ui_time_label, 5, LV_PART_MAIN);  // Dưới
lv_obj_set_style_pad_left(ui_time_label, 10, LV_PART_MAIN);   // Trái
lv_obj_set_style_pad_right(ui_time_label, 10, LV_PART_MAIN);  // Phải
```

### 1.7 Thêm Khung/Border cho Text
```cpp
// Thêm đường viền màu trắng, độ dày 2px
lv_obj_set_style_border_width(ui_time_label, 2, LV_PART_MAIN);
lv_obj_set_style_border_color(ui_time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

// Thêm bóng mờ phía sau
lv_obj_set_style_shadow_width(ui_time_label, 5, LV_PART_MAIN);
lv_obj_set_style_shadow_color(ui_time_label, lv_color_hex(0x000000), LV_PART_MAIN);
lv_obj_set_style_shadow_ofs_x(ui_time_label, 3, LV_PART_MAIN);
lv_obj_set_style_shadow_ofs_y(ui_time_label, 3, LV_PART_MAIN);
```

---

## ⏰ 2. Tùy Chỉnh Kim Đồng Hồ (Hour, Minute, Second Hands)

### 2.1 Tùy Chỉnh Kim Giờ (Hour Hand)

**Vị trí**: `desktop.cc` → hàm `CreateAnalogClock()`

```cpp
// Tìm đoạn tạo hour_hand_:

// ========== THAY ĐỔI KÍCH THƯỚC KIM ==========
lv_obj_set_size(hour_hand_, 4, 60);  // (width, height)
// width:  độ dày (4px = mảnh, 6px = dày hơn, 8px = rất dày)
// height: chiều dài (60px = ngắn, 100px = vừa, 150px = dài)

// ========== THAY ĐỔI MÀU KIM ==========
lv_obj_set_style_bg_color(hour_hand_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
// Hiện tại: 0xFFFFFF (trắng)
// Đổi sang: 0xFFFF00 (vàng), 0xFF0000 (đỏ), v.v.

// ========== THAY ĐỔI ĐỘ CỐN MÙI VỊ (Opacity) ==========
lv_obj_set_style_bg_opa(hour_hand_, LV_OPA_100, LV_PART_MAIN);
// LV_OPA_100   = Mờ đục 100% (hiện tại)
// LV_OPA_50    = Bán mờ 50%
// LV_OPA_30    = Mờ 30%
```

### 2.2 Tùy Chỉnh Kim Phút (Minute Hand)

```cpp
// Tìm đoạn tạo minute_hand_:

// Kích thước
lv_obj_set_size(minute_hand_, 4, 80);  // width: 4px, height: 80px

// Màu
lv_obj_set_style_bg_color(minute_hand_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

// Opacity
lv_obj_set_style_bg_opa(minute_hand_, LV_OPA_100, LV_PART_MAIN);
```

### 2.3 Tùy Chỉnh Kim Giây (Second Hand)

```cpp
// Tìm đoạn tạo second_hand_:

// Kích thước
lv_obj_set_size(second_hand_, 4, 100);  // width: 4px, height: 100px

// Màu
lv_obj_set_style_bg_color(second_hand_, lv_color_hex(0xFF0000), LV_PART_MAIN);  // Đỏ

// Opacity
lv_obj_set_style_bg_opa(second_hand_, LV_OPA_100, LV_PART_MAIN);
```

### 2.4 Chỉnh Tọa Độ Kim Sao Cho Đuôi Ở Giữa Màn Hình

**Nguyên lý**: Kim được xoay quanh điểm pivot (LV_ALIGN_CENTER, 0, 0) ở giữa màn hình. Để đuôi kim luôn ở đó, cần tính đúng Y_offset.

```cpp
// Công thức: Y_offset = -height / 2

// Kim giờ (height = 60)
lv_obj_align(hour_hand_, LV_ALIGN_CENTER, 0, -30);     // -60/2 = -30

// Kim phút (height = 80)
lv_obj_align(minute_hand_, LV_ALIGN_CENTER, 0, -40);   // -80/2 = -40

// Kim giây (height = 100)
lv_obj_align(second_hand_, LV_ALIGN_CENTER, 0, -50);   // -100/2 = -50
```

**Ví dụ**: Nếu bạn muốn kim giây dài hơn (height = 120):
```cpp
lv_obj_set_size(second_hand_, 4, 120);  // Dài 120px
lv_obj_align(second_hand_, LV_ALIGN_CENTER, 0, -60);   // Y_offset = -120/2 = -60
```

### 2.5 Tùy Chỉnh Vòng Tròn Ở Giữa (Center Circle)

```cpp
// Tìm đoạn tạo center_circle_:

// Thay đổi kích thước (đường kính)
lv_obj_set_size(center_circle_, 10, 10);  // 10x10px

// Thay đổi màu
lv_obj_set_style_bg_color(center_circle_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);  // Trắng

// Thay đổi opacity
lv_obj_set_style_bg_opa(center_circle_, LV_OPA_100, LV_PART_MAIN);
```

---

## 🖼️ 3. Tùy Chỉnh Hình Ảnh Nền (Background Image)

### 3.1 Thay Đổi Kích Thước Hình Ảnh

**Vị trí**: `desktop.cc` → hàm `CreateScreensaver()`

```cpp
// Dòng hiện tại:
if (background.data != nullptr) {
    DisplayImageBackground(screensaver_, &background, 240, 240, 0);
}

// Tham số:
// Param 1: screensaver_     → object cha
// Param 2: &background      → con trỏ hình ảnh
// Param 3: 240              → chiều rộng (width)
// Param 4: 240              → chiều cao (height)
// Param 5: 0                → z-index (0 = phía sau)

// Ví dụ: phóng to gấp đôi
DisplayImageBackground(screensaver_, &background, 480, 480, 0);

// Ví dụ: sử dụng toàn bộ màn hình (giả sử màn hình 240x240)
DisplayImageBackground(screensaver_, &background, 240, 240, 0);
```

### 3.2 Tùy Chỉnh Căn Chỉnh Hình Ảnh

**Vị trí**: `desktop.cc` → hàm `DisplayImageBackground()`

```cpp
// Tìm dòng:
lv_obj_align(img_obj, LV_ALIGN_CENTER, 0, 0);

// Các vị trí căn chỉnh:
LV_ALIGN_CENTER               // Giữa màn hình (hiện tại)
LV_ALIGN_TOP_MID              // Trên-giữa
LV_ALIGN_BOTTOM_MID           // Dưới-giữa
LV_ALIGN_LEFT_MID             // Giữa-trái
LV_ALIGN_RIGHT_MID            // Giữa-phải
LV_ALIGN_TOP_LEFT             // Trên-trái
LV_ALIGN_TOP_RIGHT            // Trên-phải
LV_ALIGN_BOTTOM_LEFT          // Dưới-trái
LV_ALIGN_BOTTOM_RIGHT         // Dưới-phải

// Ví dụ: đặt ở trên-trái
lv_obj_align(img_obj, LV_ALIGN_TOP_LEFT, 0, 0);

// Ví dụ: đặt ở dưới-giữa, cách dưới 10px
lv_obj_align(img_obj, LV_ALIGN_BOTTOM_MID, 0, -10);
```

### 3.3 Tùy Chỉnh Stretch Hình Ảnh

```cpp
// Dòng hiện tại:
lv_image_set_inner_align(img_obj, LV_IMAGE_ALIGN_STRETCH);

// Các tùy chọn:
LV_IMAGE_ALIGN_STRETCH  // Kéo giãn để vừa khít (hiện tại)
LV_IMAGE_ALIGN_CENTER   // Giữa nguyên, căn giữa
LV_IMAGE_ALIGN_TOP      // Căn trên
LV_IMAGE_ALIGN_BOTTOM   // Căn dưới
LV_IMAGE_ALIGN_LEFT     // Căn trái
LV_IMAGE_ALIGN_RIGHT    // Căn phải
```

### 3.4 Tùy Chỉnh Z-Index Hình Ảnh

```cpp
// Dòng hiện tại:
lv_obj_move_to_index(img_obj, index);

// index = 0: hình ảnh phía sau (hiện tại)
// index = 5: hình ảnh ở giữa
// index = 10: hình ảnh phía trước (đè lên text/kim)

// Ví dụ: đẩy hình ảnh phía sau
lv_obj_move_to_index(img_obj, 0);

// Ví dụ: đưa hình ảnh lên trước text
lv_obj_move_to_index(img_obj, 5);
```

### 3.5 Thêm Hiệu Ứng Opacity Cho Hình Ảnh

```cpp
// Thêm vào hàm DisplayImageBackground(), sau khi set size:
lv_obj_set_style_opa(img_obj, LV_OPA_100, LV_PART_MAIN);

// Giá trị opacity:
LV_OPA_100  // 100% (hiển thị đầy đủ)
LV_OPA_80   // 80% (hơi mờ)
LV_OPA_50   // 50% (bán mờ)
LV_OPA_30   // 30% (rất mờ)
```

---

## 📌 Tóm Tắt Các Hàm Thay Đổi Hay Dùng

```cpp
// ===== TEXT =====
lv_obj_set_style_text_color(obj, color, part);      // Đổi màu chữ
lv_obj_set_style_text_font(obj, font, part);        // Đổi font
lv_obj_set_style_transform_scale(obj, scale, part); // Phóng to/thu nhỏ
lv_obj_align(obj, align, x, y);                     // Chỉnh vị trí
lv_obj_set_style_pad_all(obj, pad, part);           // Thêm khoảng cách

// ===== KIM / ĐỐI TƯỢNG =====
lv_obj_set_size(obj, width, height);                // Thay đổi kích thước
lv_obj_set_style_bg_color(obj, color, part);        // Đổi màu nền
lv_obj_set_style_bg_opa(obj, opa, part);            // Đổi độ mờ

// ===== HÌNH ẢNH =====
DisplayImageBackground(parent, image, w, h, idx);  // Thêm hình ảnh
lv_obj_align(obj, align, x, y);                     // Căn chỉnh
lv_image_set_inner_align(obj, align);               // Stretch/căn chỉnh nội dung
lv_obj_move_to_index(obj, index);                   // Chỉnh Z-index
```

---

## 🔴 Ghi Chú Quan Trọng

1. **Giá trị Rotation trong LVGL**: Dùng đơn vị **0.1 độ**
   - 900 = 90 độ
   - 3600 = 360 độ
   
2. **Pivot Point**: Kim xoay quanh pivot (đuôi kim ở trung tâm). Công thức Y_offset = -height/2

3. **DisplayLockGuard**: Mọi thay đổi UI phải được bảo vệ bằng `DisplayLockGuard lock(lcd_display_);`

4. **UpdateTimeLabel()**: Nếu đổi định dạng, nhớ cập nhật hàm này

5. **Scale**: Giá trị 256 = 100%, không phải 100 = 100%
