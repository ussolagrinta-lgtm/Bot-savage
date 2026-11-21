require("dotenv").config();

function requireEnv(name, optional = false) {
  const v = process.env[name];
  if (!v && !optional) {
    return null;
  }
  return v ?? null;
}

const config = {
  token: requireEnv("TOKEN"),
  port: Number(process.env.PORT) || 3000,
  guildId: requireEnv("GUILD_ID", true),
  applicationId: requireEnv("APPLICATION_ID", true),
  shopChannelId: requireEnv("SHOP_CHANNEL_ID", true),
  ticketCategoryId: requireEnv("TICKET_CATEGORY_ID", true),
  staffRoleId: requireEnv("STAFF_ROLE_ID", true),
  ownerUserIds: (process.env.OWNER_USER_IDS || "")
    .split(",")
    .map((s) => s.trim())
    .filter(Boolean),
  shopTitle: process.env.SHOP_TITLE || "SAVAGE — Product Shop",
  shopBanner: process.env.SHOP_BANNER_URL || null,
  accentColor: parseInt(process.env.SHOP_COLOR || "0x5865F2", 16),
};

function validateConfig() {
  const missing = [];
  if (!config.token) missing.push("TOKEN");
  return missing;
}

module.exports = { config, validateConfig };