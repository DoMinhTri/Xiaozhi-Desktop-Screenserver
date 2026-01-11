#ifndef DESKTOP_H
#define DESKTOP_H

#include <lvgl.h>
#include <memory>
#include <functional>

// Forward declarations
class LcdDisplay;
class Application;

/**
 * @class Desktop
 * @brief Quản lý screen saver khi thiết bị vào chế độ sleep/wake
 * 
 * Khi vào light sleep mode, chuyển sang screen saver với màu xanh và chữ "hello"
 * Khi wake up, quay lại screen bình thường
 */
class Desktop {
public:
    /**
     * @brief Constructor
     * @param application Con trỏ đến Application để lấy DeviceState
     * @param lcd_display Con trỏ đến LcdDisplay để quản lý màn hình
     */
    Desktop(Application* application, LcdDisplay* lcd_display);
    
    ~Desktop();

    /**
     * @brief Bắt đầu quản lý screen saver
     */
    void Start();

    /**
     * @brief Dừng quản lý screen saver
     */
    void Stop();

    /**
     * @brief Cập nhật trạng thái dựa trên DeviceState
     * Nếu DeviceState == kDeviceStateIdle → hiển thị screen saver đen
     * Nếu không → ẩn screen saver
     */
    void Update();

    /**
     * @brief Kiểm tra xem có đang ở chế độ sleep (hiển thị screen saver) không
     */
    bool IsSleeping() const { return is_sleeping_; }

private:
    Application* application_;
    LcdDisplay* lcd_display_;
    bool is_sleeping_ = false;
    
    // LVGL screen objects
    lv_obj_t* screensaver_ = nullptr;          // Screen saver
    lv_obj_t* normal_screen_ = nullptr;        // Normal screen (được lưu để restore)
    lv_obj_t* ui_time_label = nullptr;         // Time label displayed on screensaver
    
    // Analog clock objects
    lv_obj_t* analog_clock_bg_ = nullptr;      // Vòng tròn nền đồng hồ
    lv_obj_t* hour_hand_ = nullptr;            // Kim giờ
    lv_obj_t* minute_hand_ = nullptr;          // Kim phút
    lv_obj_t* second_hand_ = nullptr;          // Kim giây
    lv_obj_t* center_circle_ = nullptr;        // Vòng tròn tâm đè lên các kim

    void UpdateTimeLabel();
    void CreateTimeLabel();
    void DestroyTimeLabel();
    
    void CreateAnalogClock();
    void UpdateClockHands();
    void DestroyAnalogClock();

    /**
     * @brief Tạo screen saver với màu xanh và chữ "hello"
     */
    void CreateScreensaver();

    /**
     * @brief Hiển thị screen saver (chuyển sang screen saver)
     */
    void ShowScreensaver();

    /**
     * @brief Ẩn screen saver (quay lại screen bình thường)
     */
    void HideScreensaver();

    /**
     * @brief Xóa screen saver
     */
    void DestroyScreensaver();

    /**
     * @brief Hiển thị ảnh trên object với kích thước và vị trí tùy chỉnh
     * @param parent Object cha chứa ảnh
     * @param image Ảnh cần hiển thị (lv_image_dsc_t*)
     * @param width Chiều rộng ảnh (pixel)
     * @param height Chiều cao ảnh (pixel)
     * @param index Vị trí z-index (0 = phía sau, lớn hơn = phía trước)
     * @return Pointer đến image object được tạo, hoặc nullptr nếu thất bại
     */
    static lv_obj_t* DisplayImageBackground(lv_obj_t* parent, const lv_image_dsc_t* image,
                                            int width, int height, int index = 0);

    /**
     * @brief Callback được gọi khi vào light sleep mode
     */
    void OnEnterSleep();

    /**
     * @brief Callback được gọi khi thoát light sleep mode
     */
    void OnExitSleep();
};

#endif // DESKTOP_H


