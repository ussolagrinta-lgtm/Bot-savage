# Bot Savage — Product shop (Discord)

Discord bot that posts a **SELLING** catalog and opens a private ticket per product:

| Product | Notes |
|--------|--------|
| Source — User Mode | Budget / destructive feature tier |
| Source — Kernel Mode | Kernel source package |
| Streamproof Usermode (NVIDIA & AMD) | GPU capture hiding |
| **0/72 Method DLL — FiveM** | FiveM SKU; build confirmed in ticket |

Delivery of binaries and sources is **manual in tickets**. This repo only runs the storefront bot — not cheat binaries, drivers, or bypass code.

The Joker mechanic RP bot from the original zip lives in [`mechanic/`](mechanic/) if you still need it.

## Setup

1. Copy `.env.example` to `.env` and fill IDs (see below).
2. `npm install`
3. `npm start`

Health: `http://localhost:3000/health`

## Discord IDs

| Variable | Purpose |
|----------|---------|
| `TOKEN` | Bot token |
| `APPLICATION_ID` | Application ID (for slash commands) |
| `GUILD_ID` | Your server ID |
| `SHOP_CHANNEL_ID` | Channel where the shop panel is posted on startup |
| `TICKET_CATEGORY_ID` | Category for purchase tickets |
| `STAFF_ROLE_ID` | Role pinged on new tickets |
| `OWNER_USER_IDS` | Comma-separated user IDs allowed to use `/post-shop` |

## Commands

- `/post-shop` — Staff: post the catalog + product dropdown in the current channel.
- `/products` — Ephemeral catalog view.

Customers use the **dropdown** on the shop panel to open a ticket.

## Railway

1. Push this repo and connect on Railway.
2. Set all env vars from `.env.example`.
3. Deploy; logs should show login and health server on the assigned port.

## Edit products

Edit `products.js` (names, descriptions, price notes). Restart the bot after changes.
