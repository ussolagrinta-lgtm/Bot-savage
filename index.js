const {
  Client,
  GatewayIntentBits,
  SlashCommandBuilder,
  REST,
  Routes,
  ChannelType,
  PermissionsBitField,
  ActionRowBuilder,
  StringSelectMenuBuilder,
  ButtonBuilder,
  ButtonStyle,
  EmbedBuilder,
} = require("discord.js");
const express = require("express");

const { config, validateConfig } = require("./config");
const { PRODUCTS, getProduct } = require("./products");

const missing = validateConfig();
if (missing.length) {
  console.error(`Missing env: ${missing.join(", ")}`);
  process.exit(1);
}

const app = express();
app.get("/", (_req, res) => res.send("Savage Shop Bot Online"));
app.get("/health", (_req, res) => res.json({ ok: true }));
app.listen(config.port, () => console.log(`Health server on port ${config.port}`));

const client = new Client({
  intents: [GatewayIntentBits.Guilds, GatewayIntentBits.GuildMembers],
});

function isStaff(member) {
  if (config.ownerUserIds.includes(member.user.id)) return true;
  if (config.staffRoleId && member.roles.cache.has(config.staffRoleId)) return true;
  return member.permissions.has(PermissionsBitField.Flags.Administrator);
}

function buildShopEmbed() {
  const lines = PRODUCTS.map((p) => {
    const tag = p.tags?.length ? ` · \`${p.tags.join(", ")}\`` : "";
    return `**${p.name}**${tag}\n> ${p.description}\n> *${p.priceNote}*`;
  });

  const embed = new EmbedBuilder()
    .setTitle(config.shopTitle)
    .setDescription(
      [
        "**SELLING**",
        "",
        ...lines,
        "",
        "Pick a product below to open a private **purchase ticket**.",
      ].join("\n")
    )
    .setColor(config.accentColor)
    .setFooter({ text: "FiveM · Usermode · Kernel · Streamproof" });

  if (config.shopBanner) {
    embed.setImage(config.shopBanner);
  }

  return embed;
}

function buildProductSelectRow() {
  const menu = new StringSelectMenuBuilder()
    .setCustomId("shop_select_product")
    .setPlaceholder("Select a product to buy…")
    .addOptions(
      PRODUCTS.map((p) => ({
        label: p.shortLabel.slice(0, 100),
        description: p.category.slice(0, 100),
        value: p.id,
      }))
    );

  return new ActionRowBuilder().addComponents(menu);
}

async function createPurchaseTicket(guild, user, product) {
  if (!config.ticketCategoryId) {
    throw new Error("TICKET_CATEGORY_ID is not set");
  }

  const channelName = `buy-${product.id.replace(/_/g, "-")}-${user.username}`
    .toLowerCase()
    .replace(/[^a-z0-9-]/g, "")
    .slice(0, 100);

  const overwrites = [
    {
      id: guild.id,
      deny: [PermissionsBitField.Flags.ViewChannel],
    },
    {
      id: user.id,
      allow: [
        PermissionsBitField.Flags.ViewChannel,
        PermissionsBitField.Flags.SendMessages,
        PermissionsBitField.Flags.ReadMessageHistory,
        PermissionsBitField.Flags.AttachFiles,
      ],
    },
    {
      id: client.user.id,
      allow: [
        PermissionsBitField.Flags.ViewChannel,
        PermissionsBitField.Flags.SendMessages,
        PermissionsBitField.Flags.ManageChannels,
      ],
    },
  ];

  if (config.staffRoleId) {
    overwrites.push({
      id: config.staffRoleId,
      allow: [
        PermissionsBitField.Flags.ViewChannel,
        PermissionsBitField.Flags.SendMessages,
        PermissionsBitField.Flags.ReadMessageHistory,
      ],
    });
  }

  const channel = await guild.channels.create({
    name: channelName,
    type: ChannelType.GuildText,
    parent: config.ticketCategoryId,
    topic: `Purchase ticket · ${product.id} · ${user.id}`,
    permissionOverwrites: overwrites,
  });

  const closeRow = new ActionRowBuilder().addComponents(
    new ButtonBuilder()
      .setCustomId(`ticket_close_${channel.id}`)
      .setLabel("Close ticket")
      .setStyle(ButtonStyle.Danger)
  );

  await channel.send({
    content: config.staffRoleId ? `<@${user.id}> <@&${config.staffRoleId}>` : `<@${user.id}>`,
    embeds: [
      new EmbedBuilder()
        .setTitle(`Order: ${product.name}`)
        .setDescription(
          [
            product.description,
            "",
            `**Buyer:** <@${user.id}> (\`${user.tag}\`)`,
            `**Product ID:** \`${product.id}\``,
            `**Category:** ${product.category}`,
            product.tags?.length ? `**Tags:** ${product.tags.map((t) => `\`${t}\``).join(", ")}` : "",
            "",
            "Staff: confirm game build (especially **FiveM** / **0/72**), payment, and delivery here.",
          ]
            .filter(Boolean)
            .join("\n")
        )
        .setColor(config.accentColor),
    ],
    components: [closeRow],
  });

  return channel;
}

const commands = [
  new SlashCommandBuilder()
    .setName("post-shop")
    .setDescription("Post or refresh the product shop panel (staff)"),
  new SlashCommandBuilder()
    .setName("products")
    .setDescription("List all products in chat"),
].map((c) => c.toJSON());

async function registerCommands() {
  if (!config.applicationId || !config.guildId) {
    console.warn("APPLICATION_ID or GUILD_ID missing — slash commands not registered");
    return;
  }
  const rest = new REST({ version: "10" }).setToken(config.token);
  await rest.put(Routes.applicationGuildCommands(config.applicationId, config.guildId), {
    body: commands,
  });
  console.log("Slash commands registered");
}

async function ensureShopPanel(channel) {
  const recent = await channel.messages.fetch({ limit: 20 });
  const exists = recent.some(
    (m) => m.author.id === client.user.id && m.embeds[0]?.title === config.shopTitle
  );
  if (exists) {
    console.log("Shop panel already present");
    return;
  }
  await channel.send({
    embeds: [buildShopEmbed()],
    components: [buildProductSelectRow()],
  });
  console.log("Shop panel posted");
}

client.once("ready", async () => {
  console.log(`Logged in as ${client.user.tag}`);
  try {
    await registerCommands();
  } catch (err) {
    console.error("Slash command registration failed:", err);
  }

  if (config.shopChannelId) {
    try {
      const ch = await client.channels.fetch(config.shopChannelId);
      if (ch?.isTextBased()) await ensureShopPanel(ch);
    } catch (err) {
      console.error("Could not post shop panel:", err.message);
    }
  }
});

client.on("interactionCreate", async (interaction) => {
  try {
    if (interaction.isStringSelectMenu() && interaction.customId === "shop_select_product") {
      const productId = interaction.values[0];
      const product = getProduct(productId);
      if (!product) {
        return interaction.reply({ content: "Unknown product.", ephemeral: true });
      }

      await interaction.deferReply({ ephemeral: true });
      const ticket = await createPurchaseTicket(interaction.guild, interaction.user, product);
      return interaction.editReply({
        content: `Ticket opened: ${ticket}`,
      });
    }

    if (interaction.isButton() && interaction.customId.startsWith("ticket_close_")) {
      if (!isStaff(interaction.member)) {
        return interaction.reply({ content: "Staff only.", ephemeral: true });
      }
      await interaction.reply({ content: "Closing in 3s…", ephemeral: true });
      setTimeout(() => interaction.channel?.delete().catch(() => {}), 3000);
      return;
    }

    if (!interaction.isChatInputCommand()) return;

    if (interaction.commandName === "products") {
      return interaction.reply({
        embeds: [buildShopEmbed()],
        ephemeral: true,
      });
    }

    if (interaction.commandName === "post-shop") {
      if (!isStaff(interaction.member)) {
        return interaction.reply({ content: "Staff only.", ephemeral: true });
      }
      await interaction.channel.send({
        embeds: [buildShopEmbed()],
        components: [buildProductSelectRow()],
      });
      return interaction.reply({ content: "Shop panel posted.", ephemeral: true });
    }
  } catch (err) {
    console.error("Interaction error:", err);
    if (interaction.deferred || interaction.replied) {
      await interaction.editReply({ content: "Something failed. Try again or ping staff." }).catch(() => {});
    } else {
      await interaction.reply({ content: "Something failed.", ephemeral: true }).catch(() => {});
    }
  }
});

client.login(config.token);