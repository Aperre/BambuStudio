#ifndef slic3r_wgtFilaManagerSync_h_
#define slic3r_wgtFilaManagerSync_h_

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace Slic3r {
class MachineObject;
class DevAmsTray;
} // namespace Slic3r

namespace Slic3r { namespace GUI {

class wgtFilaManagerStore;
struct FilamentSpool;

class wgtFilaManagerSync {
public:
    explicit wgtFilaManagerSync(wgtFilaManagerStore* store);
    ~wgtFilaManagerSync() = default;

    // 返回 true 表示耗材会话状态发生变化（在位 / 余量本地扣减），调用方据此刷 UI。
    bool on_device_update(MachineObject* obj);
    bool sync_all_trays(MachineObject* obj);
    bool on_device_disconnect(const std::string& dev_id, const std::string& dev_name);
    void drain_filament_hints(const std::string& dev_id);
    void dismiss_pending_badge(const std::string& dev_id,
                               const std::string& ams_id,
                               const std::string& slot_id);

private:
    const FilamentSpool* match_tray(const DevAmsTray& tray,
                                    const std::string& dev_id,
                                    const std::string& ams_id);
  
    static bool slot_pin_still_valid(const FilamentSpool& sp,
                                     const DevAmsTray&    tray);

    void check_new_filament_hint(MachineObject* obj);
    void notify_new_filament_hint(const std::string& ams_id,
                                  const std::string& slot_id,
                                  bool               show);
    bool check_print_finished_and_deduct(MachineObject* obj);
    wgtFilaManagerStore* m_store;
    std::map<std::string, bool> m_prev_tray_exists;
    std::set<std::string> m_auto_added_rfid_uuids;
    std::map<std::string, std::string> m_pending_badges;

    void check_and_register_new_rfid_spools(MachineObject* obj);
    void calibrate_pending_badges(MachineObject* obj);
    // 用户已选择"Not now"的 tray uuid 集合。
    // 命中的 uuid 在 check_new_filament_hint 中不触发角标。
    // 耗材拔出（is_exists=false）时，通过 m_slot_skipped_uuid 反查并移除。
    std::set<std::string>        m_skipped_uuids;
    // key 格式与 m_prev_tray_exists 相同；value 为该槽位当前被 skip 的 uuid。
    std::map<std::string, std::string> m_slot_skipped_uuid;
    std::map<std::string, std::string> m_prev_print_status;
};

}} // namespace Slic3r::GUI

#endif // slic3r_wgtFilaManagerSync_h_
