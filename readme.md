
# Webový server zobrazující směrovací tabulku

## Přehled
Tento projekt implementuje TCP server, který zobrazuje směrovací tabulku hostitelského počítače (v prostředí GNS3 emulátoru na zařízení typu `psi-base-node`). Směrovací tabulka je načtena z virtuálního souboru `/proc/net/route` a následně je zobrazena jako HTML tabulka ve webovém prohlížeči.

## Funkce
- Server naslouchá na portu `8080`.
- Směrovací tabulku načítá ze souboru `/proc/net/route`.
- Výstupem je HTML stránka se směrovací tabulkou obsahující následující sloupce:
  - **Interface** – identifikátor síťového rozhraní.
  - **Destination** – cílová IP adresa (např. 172.20.0.0).
  - **Mask** – maska sítě ve formátu CIDR (např. 255.255.0.0).
  - **Gateway** – IP adresa směrovače (např. 172.20.1.254).
  - **Flags** – příznaky popisující stav směrovacího záznamu.
  - **Metric** – metrika směrování (celé číslo).

- Zobrazují se pouze "použitelné" záznamy, tj. ty, které mají příznak `RTF_UP`, což znamená, že jsou použitelné pro směrování.
- IP adresa směrovače je zvýrazněna tučně, pokud je maska nulová.

## Technické vlastnosti
- Implementace je postavena na BSD socketovém rozhraní v jazyce C++.
- Server je **vícevláknový**, kde pro každý nový příchozí požadavek je pomocí funkce `fork()` vytvořen nový proces, který zpracuje danou komunikaci s klientem.
- Směrovací záznamy jsou načítány z `/proc/net/route` a zpracovávány do struktury `route_entry`.
- Pro zobrazení tabulky server vytváří HTML stránku, kterou vrací klientovi prostřednictvím HTTP protokolu.

## Sestavení a spuštění
### Sestavení
Projekt využívá CMake. 
```bash
cmake -S . -B build && cmake --build build
```
### Spuštění
Po úspěšném sestavení lze server spustit následujícím příkazem:
```bash
./build/server
```

Server bude naslouchat na portu 8080 a bude možné k němu přistupovat pomocí běžného webového prohlížeče na adrese:

**Poznámka:** V prostředí GNS3 lze použít "proxy tunel" prostřednictvím služby https://localhost.run/.

## Zdroje
- [GNS3 emulátor projektu psi-example-project-1](https://home.zcu.cz/~maxmilio/PSI/psi-example-project-1.gns3project)
- [Dokumentace k BSD socketům](https://docs.freebsd.org/en/books/developers-handbook/sockets/)
