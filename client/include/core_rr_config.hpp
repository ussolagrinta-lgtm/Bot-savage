#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace rob {

// Mirrors RobRunAIO / CoreRR.cfg keys (from embedded menu + binary strings).

struct CoreRRConfig {
    bool enabled = true;
    bool show_box = true;
    bool show_skeleton = true;
    bool show_name = true;
    bool show_armor = true;
    bool show_distance = true;
    bool show_weapon = true;
    bool show_head = true;
    bool show_self = false;
    bool show_clan = true;
    bool show_police = true;
    bool show_crouch = false;
    bool show_health = false;
    bool show_vehicle_markers = false;
    bool show_vehicle_occupants = false;
    bool cham_on = false;
    bool vehicle_chams = false;

    bool show_loot = false;
    bool show_loot_nodes = false;
    bool show_containers = false;
    bool show_doors = false;
    bool show_dropped_loot = false;
    bool show_vehicle_loot = false;

    bool loot_tier_none = true;
    bool loot_tier_common = true;
    bool loot_tier_uncommon = true;
    bool loot_tier_rare = true;
    bool loot_tier_epic = true;
    bool loot_tier_legendary = true;
    bool loot_tier_exotic = true;

    bool aa_on = false;
    bool aa_show_fov = true;
    bool aa_visible = true;
    bool aa_head = false;
    bool aa_los_on = false;
    bool aa_closest_first = false;
    bool aa_target_clan = false;
    bool aa_target_police = false;
    int aa_bone = 2;
    int aa_key = 0x12; // VK_ALT — default from menu.html
    int aa_fov_px = 150;
    int aa_smooth = 5;
    int aa_dist = 0; // 0 = unlimited in UI
    float aa_aim_cal = 1.0f;

    bool radar_on = true;
    bool radar_world = false;
    int radar_range = 150;

    int max_distance = 30000;  // stored *100 in html
    int skel_distance = 15000;
    int line_w = 1;

    std::unordered_map<std::string, std::array<uint8_t, 3>> colors;
};

class CoreRRConfigStore {
public:
    explicit CoreRRConfigStore(std::filesystem::path path);

    CoreRRConfig& Mutable() { return cfg_; }
    const CoreRRConfig& Get() const { return cfg_; }

    bool Load();
    bool Save() const;

    void ApplyKey(const std::string& key, int rawValue);
    void ApplyColor(const std::string& name, uint8_t r, uint8_t g, uint8_t b);

    std::string ToJsonState() const;

private:
    std::filesystem::path path_;
    CoreRRConfig cfg_;
};

} // namespace rob