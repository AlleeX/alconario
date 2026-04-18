# Contributing to Alconario 🎮

First off — thank you for taking the time to contribute! 🌻  
Whether it's a bug fix, a new level, pixel art, music, or just a typo — every bit helps.

---

## 📋 Table of contents

- [Code of conduct](#-code-of-conduct)
- [Getting started](#-getting-started)
- [How to contribute](#-how-to-contribute)
- [Branching & commit style](#-branching--commit-style)
- [Pull request checklist](#-pull-request-checklist)
- [Asset contributions](#-asset-contributions)
- [Reporting bugs](#-reporting-bugs)
- [Suggesting features](#-suggesting-features)

---

## 🤝 Code of conduct

Be kind, be patient, be constructive.  
We follow the [Contributor Covenant v2.1](https://www.contributor-covenant.org/version/2/1/code_of_conduct/).

---

## 🛠 Getting started

1. **Fork** the repository and clone your fork:

   ```bash
   git clone https://github.com/<your-username>/alconario.git
   cd alconario
   ```

2. **Install the toolchain:**

   ```bash
   # macOS
   brew install cc65

   # Debian / Ubuntu
   sudo apt install cc65
   ```

3. **Vendor neslib:**

   ```bash
   git clone https://github.com/clbr/neslib.git lib/neslib
   ```

4. **Build** and make sure it compiles cleanly:

   ```bash
   make
   ```

5. **Run** it in an emulator (FCEUX by default):

   ```bash
   make run
   ```

---

## 🧩 How to contribute

| Area | What we need |
|---|---|
| 🐛 Bug fixes | Reproduce → fix → PR |
| 🎮 Gameplay | New mechanics, enemy types, power-ups |
| 🖼 Pixel art / CHR | Tiles, sprites, backgrounds (`assets/chr/`) |
| 🎵 Music / SFX | FamiTracker `.ftm` + famitone2 export (`assets/music/`) |
| 🗺 Levels | Nametable / collision data (`assets/levels/`) |
| 📖 Docs | README, code comments, this file |
| 🔧 Tooling | Asset pipeline scripts (`tools/`) |

---

## 🌿 Branching & commit style

```
main          — stable, always builds
dev           — integration branch, PRs target here
feature/<name>
fix/<name>
assets/<name>
docs/<name>
```

**Commit message format** (conventional commits, loosely):

```
<type>: short description (max 72 chars)

Optional longer body.
```

Types: `feat` `fix` `docs` `assets` `refactor` `chore` `style`

Examples:
```
feat: add enemy patrol AI
fix: clamp player X to screen edge correctly
assets: add village tileset CHR
docs: update build instructions for Linux
```

---

## ✅ Pull request checklist

Before opening a PR, please confirm:

- [ ] `make` completes without errors or warnings
- [ ] Code follows the existing style (tabs, snake_case, one module = one `.c`/`.h` pair)
- [ ] New C files have a matching header in `include/`
- [ ] Asset files are placed in the correct `assets/` sub-folder
- [ ] Commit history is clean (squash WIP commits)
- [ ] PR description explains *what* and *why*

---

## 🖼 Asset contributions

- **Tiles / sprites** — export as raw `.chr` (8 KiB) using NEXXT or YY-CHR and place in `assets/chr/`.
- **Palettes** — 16-byte `.pal` dumps in `assets/palettes/`.
- **Music** — include both the `.ftm` source and the famitone2-exported `.s` in `assets/music/`.
- **Levels** — binary nametable dumps or assembly includes in `assets/levels/`.

Please keep original editable sources (`.ftm`, `.png` layered files, Tiled `.tmx`) alongside the exports so others can build on your work.

---

## 🐛 Reporting bugs

Open a [GitHub Issue](../../issues/new) and include:

1. What you expected to happen.
2. What actually happened.
3. Steps to reproduce (emulator used, OS, cc65 version).
4. Screenshot / recording if possible.

---

## 💡 Suggesting features

Open a [GitHub Issue](../../issues/new) with the label `enhancement` and describe:

- The feature and its motivation.
- How it fits the game's style / story.
- Any implementation ideas you already have.

---

## ❓ Questions?

Open a Discussion or an Issue tagged `question`. We're happy to help.

---

## 👤 Authors

| Role | Name |
|---|---|
| 🎮 Creator & lead developer | **Aliaksandr Kavalenka** |
| 🤖 AI friends & pair programmers | GitHub Copilot & friends |

This project was born from the imagination of **Aliaksandr Kavalenka** and
brought to life with a little help from AI companions who never get tired of
writing linker configs at midnight. 🌙

Want to join the credits? Open a PR — contributors get listed here. 🌻

---

*Happy hacking — and long live Alconario!* 🌻🕹
