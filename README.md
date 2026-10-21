# Pou for Nintendo Wii U (Homebrew Port)

A native, dual-screen port of **Pou** for the Nintendo Wii U console.

The Wii U GamePad’s resistive touchscreen and stylus naturally emulate the original smartphone touch controls, while the 1080p TV output delivers an ambient, high-definition virtual pet experience.

---

## 🌟 Key Features

### 📺 Dual-Screen Architecture
- **TV Output (1920x1080 @ 60 FPS):**
  - Ambient room view rendered with high-resolution wallpapers and furniture.
  - Large procedural Pou avatar mirroring expressions and animations in real time.
  - High-contrast HUD gauges: **Fullness** (Apple), **Health** (Cross), **Energy** (Lightning), and **Fun** (Ball).
  - Coins counter, Level badge, and animated XP progression bar.
- **GamePad DRC (854x480 @ 60 FPS):**
  - Direct stylus interaction viewport.
  - Top navigation bar for cycling through rooms with touch arrows.
  - Contextual bottom drawers for Fridge, Potion Shelf, and Shop catalog.
  - Stylus petting and tickling: stroke Pou to produce happy smiles, purrs, and floating heart particles.

### 🏠 Rooms & Mechanics
1. **Kitchen:**
   - Fridge drawer with food carousel (Apples, Burgers, Cherries, Donuts, Cakes).
   - Drag and drop food straight into Pou's mouth.
   - Pou tracks food with its eyes and opens its mouth when food is brought close.
   - Chewing animation with sound effects; head-shake refusal when Pou is too full.
2. **Bathroom:**
   - Drag soap over Pou to lather rich foam and bubble particles.
   - Drag shower head to spray water, rinsing away soap and dirt splatters.
   - Restores hygiene and awards XP.
3. **Laboratory:**
   - Medicine shelf featuring Health Potions, Small Health, Energizers, Hunger Potions, and Fat Burners.
   - Drag potions to Pou's mouth to drink and instantly modify pet stats.
4. **Bedroom:**
   - Nightstand lamp: tap to toggle sleep mode.
   - Night mode darkens screens, puts Pou to sleep, emits floating "Zzz" particles, and rapidly regenerates energy.
   - Wardrobe / Closet to cycle wallpapers.
5. **Game Room:**
   - Physics-based bouncing ball (flick/kick around the room with stylus).
   - Joystick button launches the Mini-Games selection menu.

### 🎮 4 Complete Mini-Games
1. **Food Drop:**
   - Slide Pou across the bottom to catch delicious falling foods while dodging trash (shoes, cloths, horseshoes).
2. **Sky Fall:**
   - Sky dive downwards while dodging oncoming clouds using the GamePad **Gyroscope / Accelerometer tilt**, Left Stick, or Stylus drag.
3. **Pou Jump:**
   - Doodle Jump style vertical platformer. Steer Pou across moving, fragile, and space platforms with screen wrapping and altitude tracking.
4. **Pou Match:**
   - 7x7 Match-3 puzzle game. Tap and swap adjacent colored Pou faces to clear matches, trigger cascading drop combos, and beat the 60-second timer.

---

## 📁 Repository Structure

```
wiiu-pou/
├── Makefile                 # devkitPPC + WUT build system
├── CMakeLists.txt           # Modern CMake configuration (Wii U & PC simulation)
├── README.md                # Documentation & installation guide
├── code/
│   ├── app.xml              # Title ID (0005000010504F55) & OS config
│   ├── cos.xml              # FSA filesystem & system permissions
│   └── Pou.rpx              # Compiled Wii U executable
├── content/
│   └── assets/              # Extracted authentic game assets
│       ├── fonts/           # Cookies.ttf
│       ├── images/          # Wallpapers, foods, potions, games, icons, etc.
│       ├── musics/          # 4 Mini-game music tracks (.wav)
│       └── sounds/          # 23 Sound effect clips (.wav)
├── meta/
│   ├── meta.xml             # Channel name & multi-language titles
│   ├── iconTex.tga          # 128x128 uncompressed TGA channel icon
│   ├── bootTvTex.tga        # 1920x1080 uncompressed TGA TV banner
│   ├── bootDrcTex.tga       # 854x480 uncompressed TGA GamePad banner
│   └── bootSound.btsnd      # Boot jingle audio
├── tools/
│   ├── generate_meta_assets.ps1  # Automated TGA banners/icon generator
│   ├── convert_boot_sound.ps1   # wav2btsnd audio conversion helper
│   ├── package_wup.ps1          # NUSPacker packaging script (PowerShell)
│   └── package_wup.sh           # NUSPacker packaging script (Bash)
└── src/
    ├── main.cpp             # Dual-screen SDL2 loop, WHB & VPAD handling
    ├── core/                # State machine, input manager, resource cache
    ├── entities/            # Procedural Pou avatar & physics ball
    ├── rooms/               # Kitchen, Bathroom, Lab, Bedroom, Game Room, Shop
    ├── games/               # Food Drop, Sky Fall, Pou Jump, Pou Match
    └── ui/                  # TV and GamePad HUD gauges and navigation
```

---

## 🛠️ Build & Packaging Options

You have two main paths to compile and package Pou:

### Option A: Zero-Install Cloud Build (GitHub Actions) — Recommended!
You do **not** need to install devkitPro, WUT, or Java on your computer:
1. Push this project repository to your GitHub account.
2. The included [`.github/workflows/build.yml`](file:///.github/workflows/build.yml) automatically runs inside devkitPro's official container.
3. It installs `wut` and `wiiu-portlibs`, compiles `Pou.rpx`, and packages the title into a ready-to-install `.zip`.
4. Add your console's `WIIU_COMMON_KEY` to your repository's **Settings > Secrets > Actions** if you want automatic WUP encryption.
5. Download the pre-built `Pou_WUP_Package.zip` directly from the **Actions** tab artifacts!

---

### Option B: Local Windows Toolchain Setup

If you prefer building locally on your PC:

#### 1. devkitPro & WUT (Wii U Toolchain)
- Download the official graphical installer: [devkitProUpdater.exe](https://github.com/devkitPro/installer/releases/latest).
- Run the installer. In the component selection screen, check **Wii U Development** (installs `devkitPPC`, `wut`, and MSYS2).
- Once installed, open `C:\devkitPro\msys2\msys2.exe` and install the required SDL2 libraries:
  ```bash
  pacman -Syu
  pacman -S devkitPPC wut wiiu-portlibs wiiu-sdl2 wiiu-sdl2_image wiiu-sdl2_ttf wiiu-sdl2_mixer
  ```
- Build `Pou.rpx`:
  ```bash
  cd /c/Users/matti/.gemini/antigravity/scratch/wiiu-pou
  make
  ```

#### 2. Java Runtime (for NUSPacker)
NUSPacker requires a Java runtime. Install OpenJDK with Windows Package Manager:
```powershell
winget install EclipseAdoptium.Temurin.17.JRE
```

#### 3. NUSPacker
- `NUSPacker.jar` has already been downloaded to [`tools/NUSPacker.jar`](file:///tools/NUSPacker.jar).
- Run the packaging script:
  ```powershell
  $env:WIIU_COMMON_KEY = "<YOUR_32_HEX_CHAR_COMMON_KEY>"
  powershell -File tools/package_wup.ps1
  ```

---

## 📦 Title Packaging for WUP Installer GX2

To install Pou as a standalone channel on your Wii U System Menu:

1. **Generate Meta Banners & Audio (if modifying):**
   ```powershell
   powershell -File tools/generate_meta_assets.ps1
   ```
2. **Convert Boot Audio:**
   ```bash
   wav2btsnd -i content/assets/musics/music_food.wav -o meta/bootSound.btsnd -l 0
   ```
3. **Run NUSPacker:**
   ```bash
   export WIIU_COMMON_KEY="<32_HEX_CHARACTERS_COMMON_KEY>"
   ./tools/package_wup.sh . Pou_WUP_Output
   ```
   NUSPacker encrypts the directory and outputs:
   - `00000000.app`, `00000001.app`, etc.
   - `00000000.h3`, `00000001.h3`, etc.
   - `title.tmd`
   - `title.tik`
   - `title.cert`

---

## 🚀 Installation on Wii U

1. **SD Card Preparation:**
   Copy the contents of `Pou_WUP_Output` into `SD:/install/Pou/` on your FAT32 SD card:
   ```
   SD:/install/Pou/
   ├── 00000000.app
   ├── 00000000.h3
   ├── 00000001.app
   ├── title.cert
   ├── title.tik
   └── title.tmd
   ```
2. **Launch Custom Firmware:**
   Boot your Wii U with **Aroma** or **Tiramisu** enabled (required for signature patching custom Title IDs).
3. **Install via WUP Installer GX2:**
   - Launch **WUP Installer GX2**.
   - Select **Pou**.
   - Choose **Install -> NAND** or **Install -> USB**.
4. Launch **Pou** from your Wii U System Menu!

---

## 💾 Save Data
Game progress (fullness, energy, fun, health, level, coins, inventory, and room customizations) automatically persists to:
- `sd:/wiiu/apps/Pou/pou_save.dat` (on SD card)
- or `fs:/vol/save/pou_save.dat` (on internal console storage).
Auto-saves occur every 30 seconds and upon gracefully exiting via the **HOME** button.
