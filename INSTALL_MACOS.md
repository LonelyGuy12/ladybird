# Installing Ladybird on macOS

## Quick Installation

1. **Download** the `Ladybird-0.1.0-macOS-arm64.dmg` from GitHub Actions artifacts
2. **Open** the DMG file
3. **Drag** `Ladybird.app` to the `Applications` folder
4. **Done!** Find Ladybird in your Applications folder

## First Launch (Gatekeeper)

Since Ladybird is not signed with an Apple Developer certificate, macOS Gatekeeper will block it on first launch.

### Method 1: Right-Click to Open (Recommended)

1. Go to `Applications` folder
2. **Right-click** (or Control+click) on `Ladybird.app`
3. Select **"Open"** from the menu
4. Click **"Open"** in the dialog that appears
5. Ladybird will now open and remember this preference

### Method 2: System Settings

If the app won't open:

1. Try to open Ladybird normally (it will be blocked)
2. Go to **System Settings** → **Privacy & Security**
3. Scroll down to find the message about Ladybird
4. Click **"Open Anyway"**
5. Enter your password if prompted
6. Click **"Open"** in the confirmation dialog

### Method 3: Terminal Command (Advanced)

Remove the quarantine attribute:

```bash
xattr -d com.apple.quarantine /Applications/Ladybird.app
```

Then launch normally.

## Troubleshooting

### "App is damaged and can't be opened"

**Cause:** The app wasn't properly signed (older build).

**Solution:** 
- Download the latest DMG from GitHub Actions
- The newer builds include ad-hoc signing
- Or run: `codesign --force --deep --sign - /Applications/Ladybird.app`

### "Cannot verify developer"

This is normal for unsigned apps.

**Solution:** Use Method 1 or 2 above to bypass Gatekeeper.

### App doesn't appear in Applications

**Cause:** The DMG didn't mount correctly.

**Solution:**
1. Eject the DMG if mounted
2. Re-download the DMG file
3. Open and drag the app again

### App crashes on launch

**Check:**
1. You're on Apple Silicon (M1/M2/M3) - the build is arm64 only
2. macOS version 14.0 or later
3. Console.app for crash logs under "Crash Reports"

## Uninstalling

Simply drag `Ladybird.app` from Applications to Trash.

Optional: Remove application data:
```bash
rm -rf ~/Library/Application\ Support/Ladybird
rm -rf ~/Library/Caches/Ladybird
```

## Requirements

- **Architecture:** Apple Silicon (M1, M2, M3, etc.)
- **OS Version:** macOS 14.0 (Sonoma) or later
- **Disk Space:** ~500 MB

## Building from Source

If you want to build Ladybird yourself instead of using the pre-built DMG:

```bash
# Install dependencies
brew install llvm@20 cmake ninja qt@6 python@3.14

# Clone the repository
git clone https://github.com/LonelyGuy12/ladybird.git
cd ladybird

# Build
python3 Meta/ladybird.py build

# The app will be in Build/release/bin/Ladybird.app
```

## Security Note

The DMG includes an **ad-hoc signature** which means:
- ✅ Prevents "damaged app" errors
- ✅ Allows basic integrity checking
- ❌ Not notarized by Apple
- ❌ Not signed with Developer ID

This is normal for open-source builds. For a production release, a proper Apple Developer certificate would be used.

## Getting Help

- **GitHub Issues:** https://github.com/LonelyGuy12/ladybird/issues
- **Test Suite:** Open `tests_python/index.html` in Ladybird to verify functionality
- **Documentation:** See main README.md

## License

See LICENSE file in the repository.
