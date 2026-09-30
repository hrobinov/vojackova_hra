# Vojáčková hra

Tady je hra, co zkouším dělat pomocí AI.

Voják na mýtině v lese proti vlnám zombíků: chodí se šipkami nebo W A S D, střílí se myší, za zabité zombíky jsou peníze a za ty se v obchodě kupují vylepšení, rakety a brokovnice. Hry se ukládají do `%APPDATA%\VojackovaHra\saves`, nastavení do `%APPDATA%\VojackovaHra\config.ini`.

## Stažení

Po každém commitu se hra na GitHubu sama sestaví. Nejnovější verze z `master` je vždycky tady:

**[vojackova_hra.exe](https://github.com/hrobinov/vojackova_hra/releases/download/latest/vojackova_hra.exe)**

Stačí ji stáhnout a spustit, nic dalšího nepotřebuje. Sestavení z jiných větví jsou u jednotlivých běhů v [Actions](https://github.com/hrobinov/vojackova_hra/actions) (pod Artifacts).

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
