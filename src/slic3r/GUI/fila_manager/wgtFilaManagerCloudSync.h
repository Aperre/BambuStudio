#ifndef slic3r_wgtFilaManagerCloudSync_h_
#define slic3r_wgtFilaManagerCloudSync_h_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

#include "AmsAutoPushThrottle.h"
#include "wgtFilaManagerStore.h"

namespace Slic3r { namespace GUI {

class wgtFilaManagerCloudClient;
struct FilamentSpool;

class wgtFilaManagerCloudSync {
public:
    struct AmsChangedSpool {
        std::string spool_id;
        std::string tag_uid;
        int64_t     net_weight {0};
    };

    using AutoPushSummaryFn = std::function<void(const nlohmann::json&)>;

    wgtFilaManagerCloudSync(wgtFilaManagerStore* store, wgtFilaManagerCloudClient* client);
    ~wgtFilaManagerCloudSync() = default;

    void pull_from_cloud();
    void push_spool_to_cloud(const std::string& spool_id);
    void push_update_to_cloud(const std::string& spool_id, const nlohmann::json& local_patch);
    void push_delete_to_cloud(const std::vector<std::string>& spool_ids);
    void fetch_filament_config(std::function<void(const nlohmann::json&)> on_done);

    void notify_ams_synced(const std::vector<AmsChangedSpool>& changed,
                           AmsAutoPushThrottle::DeviceState   device_state);

    void sync_ams_to_cloud(const std::string& dev_id,
                           const std::vector<std::string>& spool_ids);

    void sync_slot_mappings_to_cloud(const std::string& dev_id,
                                     const std::vector<EjectedSlotSnapshot>& ejected);

    void sync_slot_bindings_to_cloud(const std::string& dev_id,
                                     const std::vector<std::string>& spool_ids,
                                     bool is_bind);

    void push_all_now();

    AmsAutoPushThrottle&       throttle()       { return m_throttle; }
    const AmsAutoPushThrottle& throttle() const { return m_throttle; }

    void set_on_auto_push_summary(AutoPushSummaryFn fn) { m_on_auto_push_summary = std::move(fn); }

    bool is_syncing() const { return m_syncing; }
    bool last_pull_succeeded() const { return m_last_pull_succeeded; }
    int last_pull_error_code() const { return m_last_pull_error_code; }
    const std::string& last_pull_error_message() const { return m_last_pull_error_message; }
    static nlohmann::json spool_to_cloud_json(const FilamentSpool& spool);
    static nlohmann::json spool_to_cloud_update_patch(const nlohmann::json& local_patch);
    static nlohmann::json spool_to_cloud_update_json(const FilamentSpool& spool, const nlohmann::json& local_patch);
    static FilamentSpool cloud_json_to_spool(const nlohmann::json& j);

private:
    // GitHub #11937: list_spools() is paginated by the cloud API (default
    // limit=20 — see wgtFilaManagerCloudClient::list_spools). A single
    // unpaginated call silently truncated any library with more than 20
    // spools, which also meant the setting_id migration below it never saw
    // spool #21 onward. fetch_all_spool_pages() walks every page (accumulating
    // into `accumulated`) before handing the full merged list to
    // merge_pulled_spools().
    void fetch_all_spool_pages(int offset, std::shared_ptr<nlohmann::json> accumulated);
    void merge_pulled_spools(const nlohmann::json& list);

    wgtFilaManagerStore*       m_store;
    wgtFilaManagerCloudClient* m_client;
    bool                       m_syncing = false;
    bool                       m_last_pull_succeeded = false;
    int                        m_last_pull_error_code = 0;
    std::string                m_last_pull_error_message;

    AmsAutoPushThrottle        m_throttle;
    AutoPushSummaryFn          m_on_auto_push_summary;
};

}} // namespace Slic3r::GUI

#endif // slic3r_wgtFilaManagerCloudSync_h_
