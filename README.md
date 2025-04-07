# eFAT – Embedded FAT Filesystem (C99)

🧩 A clean, readable FAT filesystem implementation written in C99 for 32-bit embedded systems, with complete Doxygen documentation.

---

## ✨ Why eFAT?

This project started as a refactor of legacy FAT implementations — aiming for:

- 📖 **Better readability** for maintainers and integrators  
- ⚙️ **Simple integration** in resource-constrained embedded systems  
- 🧪 **Full documentation** using Doxygen for long-term maintainability  

---

## 📚 Documentation

➡️ [Browse the full Doxygen docs](https://emmanuel-io.github.io/eFAT/index.html)

Includes API reference, configuration, architecture, and integration examples.

---

## ⚡ Features

- FAT12 / FAT16 / FAT32 support  
- Lightweight codebase (C99, no dynamic allocation)  
- Platform-agnostic I/O abstraction layer  
- Ready for bare-metal or RTOS-based systems  
- Compatible with GCC and ARM toolchains  

---

## 🔧 Platform Requirements

- C99-compatible compiler
- 32-bit architecture (tested on ARM Cortex-M)
- No heap usage (static/deterministic memory)

---

## 💡 Example Use Cases

- Embedded data loggers
- Bootloaders and firmware updaters
- SD card and removable media access
- Maintenance tools in constrained systems

---

## 🤝 Contributing

Contributions are welcome!

1. Fork the repository  
2. Create a feature branch  
3. Submit a pull request with your changes

Please open [issues](https://github.com/emmanuel-io/wp-openapi/issues) for bugs, questions, or suggestions.

---

## 📜 License

This project uses a BSD-style license.
See LICENSE for full terms and attribution (including original FatFs by ChaN).


---

## 👤 Maintainer

**Emmanuel Amadio**  
🌍 [Website](https://emmanuel-io.github.io/en)  
📫 [Email](mailto:emmanuel.amadio@gmail.com)  
🐙 [GitHub](https://github.com/emmanuel-io)

---

## 🚀 Getting Started

```c
#include "efat.h"

efat_fs fs;
efat_mount(&fs, &your_io_driver);
```
