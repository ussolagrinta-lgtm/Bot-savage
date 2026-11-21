require("dotenv").config();

const {
  Client,
  GatewayIntentBits,
  SlashCommandBuilder,
  REST,
  Routes,
  ChannelType,
  PermissionsBitField,
  ActionRowBuilder,
  ButtonBuilder,
  ButtonStyle,
  EmbedBuilder,
} = require("discord.js");

const express = require("express");

const TOKEN = process.env.TOKEN;
const PORT = Number(process.env.PORT) || 3000;

if (!TOKEN) {
  console.error("Missing TOKEN in environment. Set TOKEN in Railway/host variables.");
  process.exit(1);
}

const RUOLO_ID = "1474463924870058024";
const CATEGORIA_ID = "1474463973062606922";
const CLASSIFICA_ID = "1478886552187961468";
const PANNELLO_ID = "1478885140938883193";
const APPLICATION_ID = "1432354048325189757";
const GUILD_ID = "1473648022524133523";

const app = express();
app.get("/", (_req, res) => res.send("Bot Online"));
app.get("/health", (_req, res) => res.json({ ok: true }));
app.listen(PORT, () => console.log(`HTTP health server on port ${PORT}`));

const client = new Client({
  intents: [GatewayIntentBits.Guilds, GatewayIntentBits.GuildMembers],
});

let utenti = {};
const inServizio = new Set();

const commands = [
  new SlashCommandBuilder()
    .setName("fattura")
    .setDescription("Registra una fattura")
    .addStringOption((o) =>
      o.setName("tipo").setDescription("patente / riparazione / altro").setRequired(true)
    )
    .addIntegerOption((o) =>
      o.setName("prezzo").setDescription("Prezzo").setRequired(true)
    ),
].map((c) => c.toJSON());

async function registerSlashCommands() {
  const rest = new REST({ version: "10" }).setToken(TOKEN);
  await rest.put(Routes.applicationGuildCommands(APPLICATION_ID, GUILD_ID), {
    body: commands,
  });
  console.log("Slash commands registered");
}

async function ensureServicePanel(channel) {
  const recent = await channel.messages.fetch({ limit: 15 });
  const hasPanel = recent.some(
    (m) =>
      m.author.id === client.user.id &&
      m.embeds[0]?.title?.includes("Pannello Servizio Meccanici")
  );

  if (hasPanel) {
    console.log("Service panel already present, skipping post");
    return;
  }

  const embed = new EmbedBuilder()
    .setTitle("Pannello Servizio Meccanici")
    .setDescription("Premi il bottone per entrare o uscire dal servizio");

  const row = new ActionRowBuilder().addComponents(
    new ButtonBuilder()
      .setCustomId("entra_servizio")
      .setLabel("Entra in Servizio")
      .setStyle(ButtonStyle.Success),
    new ButtonBuilder()
      .setCustomId("esci_servizio")
      .setLabel("Esci dal Servizio")
      .setStyle(ButtonStyle.Danger)
  );

  await channel.send({ embeds: [embed], components: [row] });
}

client.once("ready", async () => {
  console.log(`Logged in as ${client.user.tag}`);

  try {
    await registerSlashCommands();
  } catch (err) {
    console.error("Failed to register slash commands:", err.message);
  }

  try {
    const channel = await client.channels.fetch(PANNELLO_ID);
    if (channel?.isTextBased()) {
      await ensureServicePanel(channel);
    }
  } catch (err) {
    console.error("Failed to set up service panel:", err.message);
  }
});

client.on("interactionCreate", async (interaction) => {
  try {
    if (interaction.isButton()) {
      if (!interaction.member.roles.cache.has(RUOLO_ID)) {
        return interaction.reply({ content: "Non sei Meccanico", ephemeral: true });
      }

      if (interaction.customId === "entra_servizio") {
        inServizio.add(interaction.user.id);
        return interaction.reply({ content: "Sei in servizio", ephemeral: true });
      }

      if (interaction.customId === "esci_servizio") {
        inServizio.delete(interaction.user.id);
        return interaction.reply({ content: "Sei uscito dal servizio", ephemeral: true });
      }
    }

    if (!interaction.isChatInputCommand()) return;

    if (!interaction.member.roles.cache.has(RUOLO_ID)) {
      return interaction.reply({ content: "Non sei Meccanico", ephemeral: true });
    }

    if (!inServizio.has(interaction.user.id)) {
      return interaction.reply({ content: "Devi essere in servizio!", ephemeral: true });
    }

    const userId = interaction.user.id;

    if (!utenti[userId]) {
      utenti[userId] = { fatture: 0, totale: 0, canaleId: null };
    }

    if (interaction.commandName === "fattura") {
      const tipo = interaction.options.getString("tipo");
      const prezzo = interaction.options.getInteger("prezzo");

      utenti[userId].fatture += 1;
      utenti[userId].totale += prezzo;

      if (!utenti[userId].canaleId) {
        const canale = await interaction.guild.channels.create({
          name: `joker-${interaction.user.username}`,
          type: ChannelType.GuildText,
          parent: CATEGORIA_ID,
          permissionOverwrites: [
            {
              id: interaction.guild.id,
              deny: [PermissionsBitField.Flags.ViewChannel],
            },
            {
              id: userId,
              allow: [PermissionsBitField.Flags.ViewChannel],
            },
          ],
        });
        utenti[userId].canaleId = canale.id;
      }

      const canale = await client.channels.fetch(utenti[userId].canaleId);

      await canale.send(
        `Fattura\nTipo: ${tipo}\nPrezzo: ${prezzo}$\n\n` +
          `Totale Fatture: ${utenti[userId].fatture}\n` +
          `Totale Guadagni: ${utenti[userId].totale}$`
      );

      await interaction.reply({ content: "Fattura registrata!", ephemeral: true });
    }
  } catch (err) {
    console.error("Interaction error:", err);
    if (interaction.isRepliable() && !interaction.replied && !interaction.deferred) {
      await interaction.reply({ content: "Errore interno.", ephemeral: true }).catch(() => {});
    }
  }
});

setInterval(async () => {
  try {
    const arr = Object.entries(utenti)
      .map(([id, data]) => ({ id, ...data }))
      .sort((a, b) => b.fatture - a.fatture);

    let testo = "Classifica Giornaliera Meccanici\n\n";
    arr.forEach((u, i) => {
      testo += `${i + 1}. <@${u.id}> - ${u.fatture} fatture - ${u.totale}$\n`;
    });

    const channel = await client.channels.fetch(CLASSIFICA_ID);
    if (channel?.isTextBased()) {
      await channel.send(testo || "Nessuna fattura oggi.");
    }

    utenti = {};
  } catch (err) {
    console.error("Leaderboard reset failed:", err.message);
  }
}, 86400000);

client.login(TOKEN).catch((err) => {
  console.error("Discord login failed:", err.message);
  process.exit(1);
});