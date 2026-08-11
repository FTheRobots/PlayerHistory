# Player History

Forensic logging and admin tools for DayZ servers. The mod records what players do on the game server; the backend indexes it; the admin client lets you search, replay, and moderate from anywhere.

**Version:** 2.0.0-rc.1

---

## How it works

```
DayZ server (game box)                    Your PC / anywhere
├── PlayerHistory mod                     └── Admin client
│   writes logs to profiles/PlayerHistory       browser or portable .exe
└── PlayerHistory Server  ←────────────── connects over HTTP/WebSocket
    indexes logs, serves API
```

1. **Mod** — runs on the DayZ server, writes structured logs to `$profile:PlayerHistory/`
2. **Server** — runs on the game box, reads those logs, builds a searchable database
3. **Web client** — connects remotely to search players, view timelines, live map, and run admin commands

---

## Features

### Server recording (mod)

- **Player timelines** — sessions, joins, disconnects, respawns, kicks, bans
- **Position tracking** — movement trails with configurable distance and interval
- **Inventory** — pickup, drop, move, attach, container use
- **Combat** — damage, deaths, kills
- **Vehicles** — enter/exit, engine, doors, crashes
- **Actions** — eating, bandaging, building, doors, and other completed actions
- **Chat** — in-game messages
- **Death snapshots** — full inventory at time of death
- **Live dashboard feed** — `server.json` snapshot for online players and map
- **Admin commands** — heal, kill, teleport, spawn, ban, restore death/inventory, and more (queued from the dashboard)

All logging is toggled per category in `config.json`. See the [configuration wiki](docs/Home.md).

### Admin dashboard (web / desktop)

- **Server dashboard** — live online players, FPS, stats, last actions
- **Global map** — live player positions, heatmaps, map replay ([dayz.xam.nu](https://dayz.xam.nu/) tiles)
- **Player search & timeline** — full event history with map replay per player
- **Compare timelines** — side-by-side forensic view for multiple players
- **Item search & tracking** — find items by PID, trace movement across players
- **Advanced search** — saved queries and watchlists
- **Analytics** — server activity overview
- **Player inventory** — live and snapshot inventory views
- **Admin tools** — users, roles, audit log, bans, CFTools integration, multi-server instances
- **Real-time updates** — WebSocket push when new events arrive

### Backend (PlayerHistory Server)

- Indexes mod output into SQLite for fast search
- JWT auth with roles and fine-grained permissions
- Multi-instance support (several DayZ servers from one install)
- Event retention and automatic log cleanup
- REST API + WebSocket for the admin client

---

## Requirements

### Game server

| Requirement | Notes |
|-------------|--------|
| **DayZ dedicated server** | Windows is the primary target |
| **PlayerHistory mod** | Add to your server `-mod=` line |
| **PlayerHistory Server** | Runs on the **same machine** as DayZ (reads `profiles/PlayerHistory`) |
| **Disk space** | Depends on player count and logging settings; retention is configurable (default 14 days) |

### Admin client (your PC)

| Requirement | Notes |
|-------------|--------|
| **Modern browser** | Or the portable Windows admin `.exe` |
| **Network access** | To the game box on port **3847** (or HTTPS via reverse proxy) |
| **No DayZ install needed** | Client is remote-only |

### Development / building from source

| Requirement | Notes |
|-------------|--------|
| **Node.js 18+** | For PlayerHistory Server and Web client builds |
| **npm** | Install dependencies and run build scripts |

Portable `.exe` builds are available for the server and admin client — Node.js is not required on the game box if you use those.

---

## Quick start

### 1. Install the mod

1. Copy the `PlayerHistory` folder into your DayZ server `@mods` directory (or your mod pack).
2. Add it to your server start parameters, e.g. `-mod=@PlayerHistory;...`
3. Start the server once — the mod creates `$profile:PlayerHistory/config.json` with defaults.

### 2. Run PlayerHistory Server

On the **game server machine**:

1. Deploy `PlayerHistory-Server.exe` (or run from source — see [PlayerHistory_Server/README.md](../PlayerHistory_Server/README.md))
2. Run **`Start-Server.bat`** on first launch
3. Enter your `profiles/PlayerHistory` path, retention days, port, and owner account when prompted
4. Forward **TCP 3847** (or put HTTPS in front with nginx/Caddy)

### 3. Connect the admin client

1. Open the web client or portable admin `.exe` (see [PlayerHistory_Web/README.md](../PlayerHistory_Web/README.md))
2. Enter your server URL, e.g. `http://your-server-ip:3847`
3. Sign in with the owner account created during setup

---

## Configuration

Mod settings live in:

```
$profile:PlayerHistory/config.json
```

Restart the DayZ server after changes.

| Topic | Documentation |
|-------|----------------|
| Getting started, folder layout | [docs/Getting-Started.md](docs/Getting-Started.md) |
| Event categories (`log*` toggles) | [docs/Event-Categories.md](docs/Event-Categories.md) |
| Position & map trails | [docs/Position-Logging.md](docs/Position-Logging.md) |
| Inventory & death snapshots | [docs/Inventory-and-Snapshots.md](docs/Inventory-and-Snapshots.md) |
| Dashboard snapshot & disk I/O | [docs/Disk-IO-and-Dashboard.md](docs/Disk-IO-and-Dashboard.md) |
| Performance presets | [docs/Performance-and-Presets.md](docs/Performance-and-Presets.md) |

Server retention, auth, and networking are configured in **PlayerHistory Server** `config.json` — see [PlayerHistory_Server/README.md](../PlayerHistory_Server/README.md).

---

## Recommended dashboard settings

For a responsive live map and dashboard without meaningful lag:

```json
{
  "scanEntityCounts": 0,
  "serverSnapshotIntervalSeconds": 30
}
```

`serverSnapshotIntervalSeconds` controls how often live player positions update on the Global Map. Values of **10–30** are recommended for admin use.

---

## Project structure

| Folder | Purpose |
|--------|---------|
| **`PlayerHistory/`** | DayZ mod (this repo) |
| **`PlayerHistory_Server/`** | Backend API + indexer |
| **`PlayerHistory_Web/`** | Admin web UI + portable desktop client |

---

## Build scripts

| Component | Command |
|-----------|---------|
| Server (portable exe) | `PlayerHistory_Server/build-server.bat` |
| Web client (dev) | `PlayerHistory_Web/start-client.bat` |
| Admin desktop exe | `PlayerHistory_Web/build-desktop.bat` |

---

## License

See repository license file (if present).
