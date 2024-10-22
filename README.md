
# Webový server zobrazující směrovací tabulku

## Přehled
Tento projekt implementuje TCP server, který zobrazuje směrovací tabulku hostitelského počítače (v prostředí GNS3 emulátoru na zařízení typu `psi-base-node`). Směrovací tabulka je načtena z virtuálního souboru `/proc/net/route` a následně je zobrazena jako HTML tabulka ve webovém prohlížeči.

## Funkce
- Server naslouchá na portu `8080`.
- Směrovací tabulku načítá ze souboru `/proc/net/route`.
- Výstupem je HTML stránka se směrovací tabulkou obsahující následující sloupce:
  - **Interface** – identifikátor síťového rozhraní.
  - **Destination** – cílová IP adresa.
  - **Mask** – maska sítě (např. 255.255.0.0).
  - **Gateway** – IP adresa směrovače (zvýrazněna tučně pokud maska=0).
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
Projekt využívá CMake:
```bash
cmake -S . -B build && cmake --build build
```
### Spuštění
Po úspěšném sestavení lze server spustit následujícím příkazem:
```bash
./build/SPI_WEBSERVER
```

**Poznámka:** V prostředí GNS3 lze použít "proxy tunel" prostřednictvím služby https://localhost.run/.
```bash
./build/SPI_WEBSERVER & ssh -R 80:localhost:8080 nokey@localhost.run
```
## Příklad výstupu
<table><tbody><tr><th>Interface</th><th>Destination</th><th>Mask</th><th>Gateway</th><th>Flags</th><th>Metric</th></tr><tr><td>eth0</td><td>0.0.0.0</td><td>0.0.0.0/0</td><td><b>172.21.224.1</b></td><td>RTF_UP - Route is usable.<br>RTF_GATEWAY - Destination is a gateway.<br></td><td>0</td></tr><tr><td>eth0</td><td>172.21.224.0</td><td>255.255.240.0/20</td><td>0.0.0.0</td><td>RTF_UP - Route is usable.<br></td><td>0</td></tr></tbody></table>

## Zdroje
- [GNS3 emulátor projektu psi-example-project-1](https://home.zcu.cz/~maxmilio/PSI/psi-example-project-1.gns3project)
- [Dokumentace k BSD socketům](https://docs.freebsd.org/en/books/developers-handbook/sockets/)
