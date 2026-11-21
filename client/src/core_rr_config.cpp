#include "core_rr_config.hpp"

#include <fstream>
#include <sstream>

namespace rob {

namespace {

void Trim(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t'))
        ++i;
    if (i)
        s.erase(0, i);
}

bool ParseBool(const std::string& v) { return v == "1" || v == "true" || v == "yes"; }

} // namespace

CoreRRConfigStore::CoreRRConfigStore(std::filesystem::path path) : path_(std::move(path)) {}

bool CoreRRConfigStore::Load() {
    std::ifstream in(path_);
    if (!in)
        return false;

    std::string line;
    while (std::getline(in, line)) {
        Trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[')
            continue;
        const auto eq = line.find('=');
        if (eq == std::string::npos)
            continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        Trim(key);
        Trim(val);
        ApplyKey(key, std::stoi(val));
    }
    return true;
}

bool CoreRRConfigStore::Save() const {
    std::ofstream out(path_);
    if (!out)
        return false;

    out << "# CoreRR settings — recreated from RobRunAIO public build\n\n";
#define KV(k, v) out << k << "=" << (v) << "\n"
    KV("enabled", cfg_.enabled ? 1 : 0);
    KV("show_box", cfg_.show_box ? 1 : 0);
    KV("show_skeleton", cfg_.show_skeleton ? 1 : 0);
    KV("show_name", cfg_.show_name ? 1 : 0);
    KV("show_distance", cfg_.show_distance ? 1 : 0);
    KV("show_weapon", cfg_.show_weapon ? 1 : 0);
    KV("show_head", cfg_.show_head ? 1 : 0);
    KV("show_self", cfg_.show_self ? 1 : 0);
    KV("show_clan", cfg_.show_clan ? 1 : 0);
    KV("show_police", cfg_.show_police ? 1 : 0);
    KV("aa_on", cfg_.aa_on ? 1 : 0);
    KV("aa_show_fov", cfg_.aa_show_fov ? 1 : 0);
    KV("aa_visible", cfg_.aa_visible ? 1 : 0);
    KV("aa_head", cfg_.aa_head ? 1 : 0);
    KV("aa_los_on", cfg_.aa_los_on ? 1 : 0);
    KV("aa_closest_first", cfg_.aa_closest_first ? 1 : 0);
    KV("aa_target_clan", cfg_.aa_target_clan ? 1 : 0);
    KV("aa_target_police", cfg_.aa_target_police ? 1 : 0);
    KV("aa_bone", cfg_.aa_bone);
    KV("aa_key", cfg_.aa_key);
    KV("aa_fov_px", cfg_.aa_fov_px);
    KV("aa_smooth", cfg_.aa_smooth);
    KV("aa_dist", cfg_.aa_dist);
    KV("aa_aim_cal", static_cast<int>(cfg_.aa_aim_cal * 1000));
    KV("radar_on", cfg_.radar_on ? 1 : 0);
    KV("radar_world", cfg_.radar_world ? 1 : 0);
    KV("radar_range", cfg_.radar_range);
    KV("max_distance", cfg_.max_distance);
    KV("skel_distance", cfg_.skel_distance);
    KV("line_w", cfg_.line_w);
#undef KV
    return true;
}

void CoreRRConfigStore::ApplyKey(const std::string& key, int rawValue) {
    auto setBool = [&](bool& field) { field = rawValue != 0; };

    if (key == "enabled")
        setBool(cfg_.enabled);
    else if (key == "show_box")
        setBool(cfg_.show_box);
    else if (key == "show_skeleton")
        setBool(cfg_.show_skeleton);
    else if (key == "show_name")
        setBool(cfg_.show_name);
    else if (key == "show_armor")
        setBool(cfg_.show_armor);
    else if (key == "show_distance")
        setBool(cfg_.show_distance);
    else if (key == "show_weapon")
        setBool(cfg_.show_weapon);
    else if (key == "show_head")
        setBool(cfg_.show_head);
    else if (key == "show_self")
        setBool(cfg_.show_self);
    else if (key == "show_clan")
        setBool(cfg_.show_clan);
    else if (key == "show_police")
        setBool(cfg_.show_police);
    else if (key == "show_crouch")
        setBool(cfg_.show_crouch);
    else if (key == "show_health")
        setBool(cfg_.show_health);
    else if (key == "show_vehicle_markers")
        setBool(cfg_.show_vehicle_markers);
    else if (key == "show_vehicle_occupants")
        setBool(cfg_.show_vehicle_occupants);
    else if (key == "cham_on")
        setBool(cfg_.cham_on);
    else if (key == "vehicle_chams")
        setBool(cfg_.vehicle_chams);
    else if (key == "show_loot")
        setBool(cfg_.show_loot);
    else if (key == "show_loot_nodes")
        setBool(cfg_.show_loot_nodes);
    else if (key == "show_containers")
        setBool(cfg_.show_containers);
    else if (key == "show_doors")
        setBool(cfg_.show_doors);
    else if (key == "show_dropped_loot")
        setBool(cfg_.show_dropped_loot);
    else if (key == "show_vehicle_loot")
        setBool(cfg_.show_vehicle_loot);
    else if (key == "loot_tier_none")
        setBool(cfg_.loot_tier_none);
    else if (key == "loot_tier_common")
        setBool(cfg_.loot_tier_common);
    else if (key == "loot_tier_uncommon")
        setBool(cfg_.loot_tier_uncommon);
    else if (key == "loot_tier_rare")
        setBool(cfg_.loot_tier_rare);
    else if (key == "loot_tier_epic")
        setBool(cfg_.loot_tier_epic);
    else if (key == "loot_tier_legendary")
        setBool(cfg_.loot_tier_legendary);
    else if (key == "loot_tier_exotic")
        setBool(cfg_.loot_tier_exotic);
    else if (key == "aa_on")
        setBool(cfg_.aa_on);
    else if (key == "aa_show_fov")
        setBool(cfg_.aa_show_fov);
    else if (key == "aa_visible")
        setBool(cfg_.aa_visible);
    else if (key == "aa_head")
        setBool(cfg_.aa_head);
    else if (key == "aa_los_on")
        setBool(cfg_.aa_los_on);
    else if (key == "aa_closest_first")
        setBool(cfg_.aa_closest_first);
    else if (key == "aa_target_clan")
        setBool(cfg_.aa_target_clan);
    else if (key == "aa_target_police")
        setBool(cfg_.aa_target_police);
    else if (key == "aa_bone")
        cfg_.aa_bone = rawValue;
    else if (key == "aa_key")
        cfg_.aa_key = rawValue;
    else if (key == "aa_fov_px")
        cfg_.aa_fov_px = rawValue;
    else if (key == "aa_smooth")
        cfg_.aa_smooth = rawValue;
    else if (key == "aa_dist")
        cfg_.aa_dist = rawValue;
    else if (key == "aa_aim_cal")
        cfg_.aa_aim_cal = rawValue / 1000.f;
    else if (key == "radar_on")
        setBool(cfg_.radar_on);
    else if (key == "radar_world")
        setBool(cfg_.radar_world);
    else if (key == "radar_range")
        cfg_.radar_range = rawValue;
    else if (key == "max_distance")
        cfg_.max_distance = rawValue;
    else if (key == "skel_distance")
        cfg_.skel_distance = rawValue;
    else if (key == "line_w")
        cfg_.line_w = rawValue;
}

void CoreRRConfigStore::ApplyColor(const std::string& name, uint8_t r, uint8_t g, uint8_t b) {
    cfg_.colors[name] = {r, g, b};
}

std::string CoreRRConfigStore::ToJsonState() const {
    std::ostringstream o;
    o << '{';
    auto emitBool = [&](const char* k, bool v) { o << '"' << k << "\":" << (v ? "true" : "false") << ','; };
    auto emitInt = [&](const char* k, int v) { o << '"' << k << "\":" << v << ','; };
    emitBool("enabled", cfg_.enabled);
    emitBool("show_box", cfg_.show_box);
    emitBool("show_skeleton", cfg_.show_skeleton);
    emitBool("show_name", cfg_.show_name);
    emitBool("show_distance", cfg_.show_distance);
    emitBool("show_weapon", cfg_.show_weapon);
    emitBool("show_head", cfg_.show_head);
    emitBool("show_self", cfg_.show_self);
    emitBool("show_clan", cfg_.show_clan);
    emitBool("show_police", cfg_.show_police);
    emitBool("aa_on", cfg_.aa_on);
    emitBool("aa_show_fov", cfg_.aa_show_fov);
    emitBool("aa_visible", cfg_.aa_visible);
    emitBool("aa_head", cfg_.aa_head);
    emitBool("aa_los_on", cfg_.aa_los_on);
    emitBool("aa_closest_first", cfg_.aa_closest_first);
    emitBool("aa_target_clan", cfg_.aa_target_clan);
    emitBool("aa_target_police", cfg_.aa_target_police);
    emitInt("aa_bone", cfg_.aa_bone);
    emitInt("aa_key", cfg_.aa_key);
    emitInt("aa_fov_px", cfg_.aa_fov_px);
    emitInt("aa_smooth", cfg_.aa_smooth);
    emitInt("aa_dist", cfg_.aa_dist);
    emitInt("max_distance", cfg_.max_distance);
    emitInt("skel_distance", cfg_.skel_distance);
    emitInt("line_w", cfg_.line_w);
    emitBool("radar_on", cfg_.radar_on);
    emitBool("radar_world", cfg_.radar_world);
    emitInt("radar_range", cfg_.radar_range);
    emitBool("show_loot", cfg_.show_loot);
    emitBool("show_loot_nodes", cfg_.show_loot_nodes);
    emitBool("vehicle_chams", cfg_.vehicle_chams);
    o << "\"colors\":{}"; // colors optional
    o << '}';
    return o.str();
}

} // namespace rob