#include "desktop.h"
#include "lcd_display.h"
#include "display.h"
#include "application.h"
#include "tools/desktop/icons.h"

#include <esp_log.h>


static const char *TAG = "Desktop";

Desktop::Desktop(Application* application, LcdDisplay* lcd_display)
    : application_(application), lcd_display_(lcd_display) {
    ESP_LOGI(TAG, "Desktop constructor called");
    if (!application_) {
        ESP_LOGW(TAG, "Application is nullptr");
    }
    if (!lcd_display_) {
        ESP_LOGW(TAG, "LcdDisplay is nullptr");
    }
}

Desktop::~Desktop() {
    Stop();
    DestroyScreensaver();
}

void Desktop::Start() {
    ESP_LOGI(TAG, "Desktop Start() called");
    if (!lcd_display_) {
        ESP_LOGE(TAG, "Cannot start: LcdDisplay is nullptr");
        return;
    }

    // Tạo screen saver
    CreateScreensaver();

    ESP_LOGI(TAG, "Desktop started successfully");
}

void Desktop::Stop() {
    ESP_LOGI(TAG, "Desktop stopped");
}

void Desktop::Update() {
    if (!application_) {
        ESP_LOGE(TAG, "✗ Application is nullptr!");
        return;
    }

    // Đọc DeviceState từ Application
    auto device_state = application_->GetDeviceState();
    static DeviceState last_state = (DeviceState)-1;
    if (device_state != last_state) {
        ESP_LOGI(TAG, "DeviceState changed from %d to %d (is_sleeping=%d)", last_state, device_state, is_sleeping_);
        last_state = device_state;
    }
    
    // Nếu state là IDLE (ngủ) → hiển thị screen saver đen
    if (device_state == kDeviceStateIdle) {
        if (!is_sleeping_) {
            ESP_LOGI(TAG, "✓ DeviceState is IDLE - showing black screensaver");
            is_sleeping_ = true;
            ShowScreensaver();
        }
        // Cập nhật giờ khi đang hiển thị screensaver
        UpdateTimeLabel();
        UpdateClockHands();
    } else {
        // Nếu không phải IDLE (thức) → ẩn screen saver
        if (is_sleeping_) {
            ESP_LOGI(TAG, "✓ DeviceState is not IDLE (%d) - hiding screensaver", device_state);
            is_sleeping_ = false;
            HideScreensaver();
        }
    }
}

void Desktop::CreateScreensaver() {
    if (!lcd_display_) {
        ESP_LOGE(TAG, "Cannot create screensaver: LcdDisplay is nullptr");
        return;
    }

    if (screensaver_) {
        ESP_LOGW(TAG, "Screensaver already created, destroying old one");
        DestroyScreensaver();
    }

    DisplayLockGuard lock(lcd_display_);
    // Tạo screen saver mới
    screensaver_ = lv_obj_create(nullptr);
    if (!screensaver_) { return; }
    if (background.data != nullptr)
    {
        DisplayImageBackground(screensaver_, &background, 240, 240, 0);
    }

    // Đặt background màu đen làm fallback
    lv_obj_set_style_bg_color(screensaver_, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screensaver_, LV_OPA_COVER, LV_PART_MAIN);
    // Tạo analog clock
    CreateAnalogClock();
    // Tạo time label trên screensaver
    CreateTimeLabel();
}

void Desktop::ShowScreensaver() {
    if (!screensaver_) {
        ESP_LOGW(TAG, "Screensaver is nullptr, creating it now");
        CreateScreensaver();
    }

    if (screensaver_) {
        DisplayLockGuard lock(lcd_display_);
        // Lưu lại active screen hiện tại trước khi chuyển đổi
        normal_screen_ = lv_screen_active();
        ESP_LOGI(TAG, "SavedECTION normal screen: %p, loading screensaver: %p", normal_screen_, screensaver_);
        // Chuyển sang screensaver
        lv_screen_load(screensaver_);
        ESP_LOGI(TAG, "✓ Screensaver loaded and displayed (current active: %p)", lv_screen_active());
    } else {
        ESP_LOGE(TAG, "✗ Failed to show screensaver - screensaver is nullptr");
    }
}

void Desktop::HideScreensaver() {
    if (normal_screen_) { DisplayLockGuard lock(lcd_display_);
        // Quay lại screen bình thường
        ESP_LOGI(TAG, "Restoring normal screen: %p from screensaver: %p", normal_screen_, lv_screen_active());
        lv_screen_load(normal_screen_);
        ESP_LOGI(TAG, "✓ Screensaver hidden, returned to normal screen (current active: %p)", lv_screen_active());
        normal_screen_ = nullptr;
    } else {
        ESP_LOGW(TAG, "✗ normal_screen_ is nullptr, cannot restore");
    }
}

void Desktop::DestroyScreensaver() {
    if (screensaver_) {
        DisplayLockGuard lock(lcd_display_);
        DestroyAnalogClock();
        DestroyTimeLabel();
        lv_obj_del(screensaver_);
        screensaver_ = nullptr;
        normal_screen_ = nullptr;
        ESP_LOGI(TAG, "Screensaver destroyed");
    }
}

void Desktop::OnEnterSleep() {
    ESP_LOGI(TAG, "OnEnterSleep called (deprecated - use Update() instead)");
}

void Desktop::OnExitSleep() {
    ESP_LOGI(TAG, "OnExitSleep called (deprecated - use Update() instead)");
}

lv_obj_t* Desktop::DisplayImageBackground(lv_obj_t* parent, const lv_image_dsc_t* image,
                                          int width, int height, int index) {
    if (!parent) {
        ESP_LOGE(TAG, "✗ Parent object is nullptr");
        return nullptr;
    }

    if (!image) {
        ESP_LOGE(TAG, "✗ Image is nullptr");
        return nullptr;
    }

    if (image->data == nullptr) {
        ESP_LOGE(TAG, "✗ Image data is nullptr");
        return nullptr;
    }

    // 1. Tạo đối tượng ảnh
    lv_obj_t* img_obj = lv_image_create(parent);
    if (!img_obj) {
        ESP_LOGE(TAG, "✗ Failed to create image object");
        return nullptr;
    }

    // 2. Xóa bỏ khoảng cách và border mặc định
    lv_obj_set_style_pad_all(img_obj, 0, 0);
    lv_obj_set_style_border_width(img_obj, 0, 0);
    // 3. Gán nguồn ảnh
    lv_image_set_src(img_obj, image);
    // 4. Đặt kích thước
    lv_obj_set_size(img_obj, width, height);
    // 5. Căn giữa
    lv_obj_align(img_obj, LV_ALIGN_CENTER, 0, 0);
    // 6. Tự động stretch ảnh để vừa khít khung
    lv_image_set_inner_align(img_obj, LV_IMAGE_ALIGN_STRETCH);
    // 7. Đặt vị trí z-index
    lv_obj_move_to_index(img_obj, index);
    ESP_LOGI(TAG, "✓ Image displayed: %dx%d at z-index=%d", width, height, index);

    return img_obj;
}
void Desktop::CreateTimeLabel() {
    if (!screensaver_) {
        ESP_LOGE(TAG, "Cannot create time label: screensaver is nullptr");
        return;
    }

    if (ui_time_label) {
        ESP_LOGW(TAG, "Time label already exists, destroying old one");
        DestroyTimeLabel();
    }

    DisplayLockGuard lock(lcd_display_);

    // Tạo label object trên screensaver
    ui_time_label = lv_label_create(screensaver_);
    if (!ui_time_label) {
        ESP_LOGE(TAG, "Failed to create time label");
        return;
    }

    // Đặt kiểu chữ: màu trắng, kích thước lớn
    lv_obj_set_style_text_color(ui_time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_time_label, &lv_font_montserrat_14, LV_PART_MAIN);
    
    // Thêm letter spacing để chữ trông bold hơn mà vẫn giữ outline
    //lv_obj_set_style_text_letter_space(ui_time_label, 2, LV_PART_MAIN);
    //lv_obj_set_style_text_line_space(ui_time_label, 2, LV_PART_MAIN);
    
    // Thêm outline để nổi bật
    //lv_obj_set_style_text_opa(ui_time_label, LV_OPA_COVER, LV_PART_MAIN);
    //lv_obj_set_style_outline_width(ui_time_label, 2, LV_PART_MAIN);
    //lv_obj_set_style_outline_color(ui_time_label, lv_color_hex(0x000000), LV_PART_MAIN);
    
    // Phóng to chữ bằng scale (256 = 100%, 512 = 200%, 384 = 150%)
    lv_obj_set_style_transform_scale(ui_time_label, 512, LV_PART_MAIN);
    // Cập nhật thời gian ban đầu
    UpdateTimeLabel();
    // Lấy chiều rộng thực tế của text sau khi set
    int32_t label_width = lv_obj_get_self_width(ui_time_label);
    // Đặt vị trí: giữa màn hình, phía trên (x_offset = -width/2 để căn giữa)
    lv_obj_align(ui_time_label, LV_ALIGN_TOP_MID, -label_width / 2, 25);
    // Đặt z-index cao hơn background
    lv_obj_move_to_index(ui_time_label, 10);

    ESP_LOGI(TAG, "✓ Time label created successfully (width=%d)", label_width);
}

void Desktop::DestroyTimeLabel() {
    if (ui_time_label) {
        DisplayLockGuard lock(lcd_display_);
        lv_obj_del(ui_time_label);
        ui_time_label = nullptr;
        ESP_LOGI(TAG, "Time label destroyed");
    }
}

void Desktop::UpdateTimeLabel() {
    if (!ui_time_label) { return; }
    if (!lv_obj_is_valid(ui_time_label)) { return; }
    DisplayLockGuard lock(lcd_display_);
    time_t now = time(NULL);
    struct tm *info = localtime(&now);
    lv_label_set_text_fmt(ui_time_label, "%02d:%02d:%02d", info->tm_hour, info->tm_min, info->tm_sec);
    ESP_LOGI(TAG, "Updated time: %02d:%02d:%02d", info->tm_hour, info->tm_min, info->tm_sec);
}

void Desktop::CreateAnalogClock() {
    if (!screensaver_) { return; }
    DisplayLockGuard lock(lcd_display_);

    // Tạo kim giờ (màu vàng, dọc từ dưới lên trên) - ngắn nhất
    hour_hand_ = lv_obj_create(screensaver_);
    if (!hour_hand_) { return; }
    lv_obj_set_size(hour_hand_, 4, 60);  // Gấp 1.5 lần kim phút (6*1.5=9), ngắn hơn
    lv_obj_set_style_transform_pivot_x(hour_hand_, LV_PCT(50), LV_PART_MAIN);
    lv_obj_set_style_transform_pivot_y(hour_hand_, LV_PCT(100), LV_PART_MAIN);  // 100% = bottom
    // Y_offset = -height/2 = -60/2 = -30 để pivot nằm ở trung tâm
    lv_obj_align(hour_hand_, LV_ALIGN_CENTER, 0, -30);
    lv_obj_set_style_bg_color(hour_hand_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);  // Vàng
    lv_obj_set_style_bg_opa(hour_hand_, LV_OPA_100, LV_PART_MAIN);  // 100% opacity
    lv_obj_set_style_border_width(hour_hand_, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(hour_hand_, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(hour_hand_, 0, LV_PART_MAIN);

    // Tạo kim phút (màu trắng, dọc từ dưới lên trên) - trực tiếp từ screensaver
    minute_hand_ = lv_obj_create(screensaver_);
    if (!minute_hand_) { return; }
    lv_obj_set_size(minute_hand_, 4, 80);  // Nhỏ hơn kim giây, dài hơn kim giờ
    lv_obj_set_style_transform_pivot_x(minute_hand_, LV_PCT(50), LV_PART_MAIN);
    lv_obj_set_style_transform_pivot_y(minute_hand_, LV_PCT(100), LV_PART_MAIN);  // 100% = bottom
    // Y_offset = -height/2 = -80/2 = -40 để pivot nằm ở trung tâm
    lv_obj_align(minute_hand_, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_bg_color(minute_hand_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(minute_hand_, LV_OPA_100, LV_PART_MAIN);  // 100% opacity
    lv_obj_set_style_border_width(minute_hand_, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(minute_hand_, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(minute_hand_, 0, LV_PART_MAIN);

    // Tạo kim giây (màu đỏ, dọc từ dưới lên trên) - trực tiếp từ screensaver
    second_hand_ = lv_obj_create(screensaver_);
    if (!second_hand_) { return; }
    lv_obj_set_size(second_hand_, 4, 100);
    lv_obj_set_style_transform_pivot_x(second_hand_, LV_PCT(50), LV_PART_MAIN);
    lv_obj_set_style_transform_pivot_y(second_hand_, LV_PCT(100), LV_PART_MAIN);  // 100% = bottom
    // Y_offset = -height/2 = -100/2 = -50 để pivot nằm ở trung tâm
    lv_obj_align(second_hand_, LV_ALIGN_CENTER, 0, -50);
    lv_obj_set_style_bg_color(second_hand_, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(second_hand_, LV_OPA_100, LV_PART_MAIN);  // 100% opacity
    lv_obj_set_style_border_width(second_hand_, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(second_hand_, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(second_hand_, 0, LV_PART_MAIN);

    // Tạo vùng tròn tại vị trí trung tâm để đè lên các kim
    center_circle_ = lv_obj_create(screensaver_);
    if (!center_circle_) { return; }
    lv_obj_set_size(center_circle_, 10, 10);  // Đường kính 20px
    lv_obj_align(center_circle_, LV_ALIGN_CENTER, 0, 0);  // Căn giữa (account for -30 offset)
    lv_obj_set_style_bg_color(center_circle_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);  // Đen (same as background)
    lv_obj_set_style_bg_opa(center_circle_, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_border_width(center_circle_, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(center_circle_, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(center_circle_, LV_RADIUS_CIRCLE, LV_PART_MAIN);  // Tròn hoàn hảo

    // Cập nhật rotation lần đầu
    UpdateClockHands();
}

void Desktop::UpdateClockHands() {
    if (!second_hand_) { return; }

    DisplayLockGuard lock(lcd_display_);

    time_t now = time(NULL);
    struct tm *info = localtime(&now);
    int second = info->tm_sec;          // 0-59
    int minute = info->tm_min;          // 0-59
    int hour   = info->tm_hour;         // 0-23

    // Tính độ xoay cho kim giờ (LVGL dùng 0.1 độ)
    // Kim giờ: 360 độ / 12 giờ = 30 độ/giờ = 300 (0.1 độ unit) / giờ
    // Cộng thêm ảnh hưởng của phút: 30 độ / 60 phút = 0.5 độ/phút = 5 (0.1 độ unit) / phút
    int hour_rotation = ((hour % 12) * 300) + (minute * 5);
    // Áp dụng xoay cho kim giờ
    if (hour_hand_) {
        lv_obj_set_style_transform_rotation(hour_hand_, hour_rotation, LV_PART_MAIN);
        ESP_LOGI(TAG, "DEBUG: hour=%d, minute=%d, rotation=%d (0.1deg unit), actual_degrees=%d", hour, minute, hour_rotation, hour_rotation / 10);
    }

    // Tính độ xoay cho kim phút (LVGL dùng 0.1 độ)
    // Kim phút: 360 độ / 60 phút = 6 độ/phút = 60 (0.1 độ unit) / phút
    int minute_rotation = minute * 60;
    // Áp dụng xoay cho kim phút
    if (minute_hand_) {
        lv_obj_set_style_transform_rotation(minute_hand_, minute_rotation, LV_PART_MAIN);
        ESP_LOGI(TAG, "DEBUG: minute=%d, rotation=%d (0.1deg unit), actual_degrees=%d", minute, minute_rotation, minute_rotation / 10);
    }

    // Tính độ xoay cho kim giây (LVGL dùng 0.1 độ)
    // Kim giây: 360 độ / 60 giây = 6 độ/giây = 60 (0.1 độ unit) / giây
    int second_rotation = second * 60;
    // Áp dụng xoay cho kim giây
    lv_obj_set_style_transform_rotation(second_hand_, second_rotation, LV_PART_MAIN);
    ESP_LOGI(TAG, "DEBUG: second=%d, rotation=%d (0.1deg unit), actual_degrees=%d", second, second_rotation, second_rotation / 10);
}

void Desktop::DestroyAnalogClock() {
    if (second_hand_) {
        DisplayLockGuard lock(lcd_display_);
        // Xóa kim giây, kim phút, kim giờ, vùng tròn tâm
        if (second_hand_) lv_obj_del(second_hand_);
        if (minute_hand_) lv_obj_del(minute_hand_);
        if (hour_hand_) lv_obj_del(hour_hand_);
        if (center_circle_) lv_obj_del(center_circle_);
        
        second_hand_ = nullptr;
        minute_hand_ = nullptr;
        hour_hand_ = nullptr;
        center_circle_ = nullptr;
        analog_clock_bg_ = nullptr;
        ESP_LOGI(TAG, "Analog clock destroyed");
    }
}

