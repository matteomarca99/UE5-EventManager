# UltimateEventManager

<p align="center">
  <img src="https://img.shields.io/badge/Unreal%20Engine-5-black?style=for-the-badge&logo=unrealengine&logoColor=white" />
  <img src="https://img.shields.io/badge/C%2B%2B-92%25-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/Version-1.0.0-brightgreen?style=for-the-badge" />
  <img src="https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge" />
</p>

> A powerful and flexible **Event Manager System** plugin for Unreal Engine 5, designed to streamline event-driven gameplay architecture through C++ and Blueprint support.

---

## 📖 Overview

**UltimateEventManager** is a UE5 plugin that provides a centralized, data-driven event management system. It enables developers to register, dispatch, and listen to gameplay events from both C++ and Blueprints, reducing tight coupling between game systems and promoting a clean, scalable architecture.

The plugin is built around three modules:
- **Runtime** — the core event system, available in both editor and packaged builds.
- **Nodes** — custom Blueprint nodes (K2Nodes) for a seamless visual scripting experience.
- **Editor** — editor-time tooling and utilities for configuring events in-engine.

---

## ✨ Features

- 🔔 **Centralized Event Bus** — a single subsystem to manage all gameplay events.
- 🎨 **Blueprint Support** — fully exposed to the Blueprint editor via custom K2Nodes.
- 🧩 **StructUtils Integration** — leverages UE5's `StructUtils` plugin for type-safe event payloads.
- ⚡ **Runtime & Editor Modules** — separate modules for runtime logic and editor tooling.
- 📦 **Plugin Architecture** — drop-in plugin, no engine modifications required.

---

## 🗂️ Project Structure

```
UE5-EventManager/
├── Config/                         # Plugin configuration files
├── Resources/                      # Plugin icon and resources
├── Source/
│   ├── UltimateEventManager/       # Runtime module (core event system)
│   ├── UltimateEventManagerNodes/  # Blueprint K2Nodes (UncookedOnly)
│   └── UltimateEventManagerEditor/ # Editor module (tooling)
├── UltimateEventManager.uplugin    # Plugin descriptor
└── LICENSE
```

---

## 🚀 Installation

1. **Download** the latest release from the [Releases page](https://github.com/matteomarca99/UE5-EventManager/releases).
2. **Copy** the `UltimateEventManager` folder into your project's `Plugins/` directory:
   ```
   YourProject/
   └── Plugins/
       └── UltimateEventManager/
   ```
3. **Regenerate** project files (right-click the `.uproject` → *Generate Visual Studio project files*).
4. **Build** and open the project in Unreal Engine 5.
5. **Enable** the plugin via *Edit → Plugins → UltimateEventManager*.

> ⚠️ **Requirements**: Unreal Engine 5 · `StructUtils` plugin enabled

---

## 🔧 Usage

### C++

Add the module dependency to your `Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "UltimateEventManager"
});
```

### Blueprints

The plugin exposes custom Blueprint nodes via the `UltimateEventManagerNodes` module. Search for **Event Manager** nodes directly in the Blueprint editor.

---

## 📦 Modules

| Module | Type | Loading Phase | Description |
|--------|------|---------------|-------------|
| `UltimateEventManager` | Runtime | Default | Core event manager system |
| `UltimateEventManagerNodes` | UncookedOnly | Default | Custom K2 Blueprint nodes |
| `UltimateEventManagerEditor` | Editor | Default | Editor tooling & utilities |

---

## 🛠️ Dependencies

| Plugin | Required |
|--------|----------|
| `StructUtils` | ✅ Yes |

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## 👤 Author

**Matteo Marcantoni** — [GitHub](https://github.com/matteomarca99)

---

<p align="center">
  Made with ❤️ for the Unreal Engine community
</p>
