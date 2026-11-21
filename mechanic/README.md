# Joker Meccanico Bot (external host)

Discord bot for mechanic service shifts, invoices, and a daily leaderboard.

## Run locally

1. Copy `.env.example` to `.env` and set `TOKEN` (Discord bot token).
2. `npm install`
3. `npm start`

Open `http://localhost:3000/` — you should see `Bot Online`.

## Deploy on Railway (recommended)

1. Push this repo to GitHub.
2. [railway.app](https://railway.app) → **New Project** → **Deploy from GitHub** → select this repo.
3. **Variables** → add `TOKEN` with your bot token (never commit it).
4. Railway sets `PORT` automatically; the bot listens on it for health checks.
5. Deploy. Logs should show `HTTP health server on port …` and `Logged in as …`.

## Discord checklist

- Bot invite includes `applications.commands` and permissions for the target guild.
- Hard-coded IDs in `index.js` must match your server (role, category, channels, application ID).