# Vojáčková hra

Tady je hra, co zkouším dělat pomocí AI.

Zatím je hotové jen hlavní menu: **Nová hra** a **Načíst hru** (ty ještě nic nedělají), **Nastavení** (velikost rozhraní a grafika, jako v Go editoru), **O hře** a **Konec**. Nastavení se ukládá do `%APPDATA%\VojackovaHra\config.ini`.

## Z čeho je postavená

Základ je převzatý z [kovarex_go_editor](https://github.com/kovarex/kovarex_go_editor):

- [raylib](https://www.raylib.com/) na okno a vykreslování (`libraries/raylib`)
- Agui, GUI knihovna ve stylu Factoria (`libraries/Agui`)
- vzhled z Factoria: `resources/gui.png`, `resources/style.lua`, font Titillium Web a styly v `src/ui/Theme.cpp`

## Sestavení (Windows)

Potřeba je Visual Studio 2019 nebo novější (nebo jen Build Tools) s C++ nástroji.

```
fastbuild\build.cmd Debug
```

Hra se objeví v `build\Debug\vojackova_hra.exe`. Ve VS Code stačí zmáčknout **F5**: sestaví hru a spustí ji v debuggeru.
