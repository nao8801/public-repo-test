# PC-8801 I/O ポート一覧

このドキュメントは、M88エミュレータのソースコード（`src/pc88/pc88.cpp`の`ConnectDevices()`関数）から抽出したI/Oポートの一覧です。

## ポート定義の場所

すべてのI/Oポートのマッピングは `src/pc88/pc88.cpp` の `PC88::ConnectDevices()` 関数（284-583行目）で定義されています。

---

## メインCPU側 I/Oポート

### ベースシステム (Base)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0x30 | IN | `Base::in30` | DIPSW読み取り（下位ビット） |
| 0x31 | IN | `Base::in31` | DIPSW読み取り（上位ビット + BASICモード） |
| 0x40 | IN | `Base::in40` | システムステータス（垂直帰線、RTC等） |
| 0x6e | IN | `Base::in6e` | クロック設定状態 |

**実装ファイル**: `src/pc88/base.cpp`

- **0x30**: DIPSWの下位ビットを返す（`sw30`）
- **0x31**: DIPSWの上位ビットとBASICモード情報を返す（`sw31`）
- **0x40**: 垂直帰線割り込み状態、RTC割り込み状態などを返す

### CRTC (Cathode Ray Tube Controller)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0x50 | OUT | `CRTC::out` | CRTCコマンド/データ書き込み |
| 0x51 | OUT | `CRTC::out` | CRTCコマンド/データ書き込み |
| 0x50 | IN | `CRTC::getstatus` | CRTCステータス読み取り |
| 0x51 | IN | `CRTC::in` | CRTCデータ読み取り |
| 0x00 | OUT | `CRTC::pcgout` | PCG（Programmable Character Generator）データ |
| 0x01 | OUT | `CRTC::pcgout` | PCGデータ |
| 0x02 | OUT | `CRTC::pcgout` | PCGデータ |
| 0x33 | OUT | `CRTC::setkanamode` | かなモード設定 |

**実装ファイル**: `src/pc88/crtc.cpp`

### メモリ管理 (Memory)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0x31 | OUT | `Memory::out31` | メモリバンク選択（N88/N80/RAM選択） |
| 0x32 | OUT | `Memory::out32` | メモリバンク選択（EROM bank、ALU Enable等） |
| 0x32 | IN | `Memory::in32` | メモリ状態読み取り |
| 0x33 | OUT | `Memory::out33` | メモリバンク選択（EROM bank等） |
| 0x33 | IN | `Memory::in33` | メモリ状態読み取り |
| 0x34 | OUT | `Memory::out34` | ALU演算設定 |
| 0x35 | OUT | `Memory::out35` | メモリ制御 |
| 0x5c | OUT | `Memory::out5x` | GVRAM0選択 |
| 0x5d | OUT | `Memory::out5x` | GVRAM1選択 |
| 0x5e | OUT | `Memory::out5x` | GVRAM2選択 |
| 0x5f | OUT | `Memory::out5x` | RAM選択 |
| 0x5c | IN | `Memory::in5c` | GVRAM状態読み取り |
| 0x70 | OUT | `Memory::out70` | TextWindow設定（N88時有効） |
| 0x70 | IN | `Memory::in70` | TextWindow状態読み取り |
| 0x71 | OUT | `Memory::out71` | メモリ制御 |
| 0x71 | IN | `Memory::in71` | メモリ状態読み取り |
| 0x78 | OUT | `Memory::out78` | TextWindow設定（++Port70） |
| 0x99 | OUT | `Memory::out99` | CD-BIOS/other選択 |
| 0xe2 | OUT | `Memory::oute2` | 拡張RAM制御 |
| 0xe2 | IN | `Memory::ine2` | 拡張RAM状態読み取り |
| 0xe3 | OUT | `Memory::oute3` | 拡張RAMページ選択 |
| 0xe3 | IN | `Memory::ine3` | 拡張RAM状態読み取り |
| 0xf0 | OUT | `Memory::outf0` | 辞書ROMバンク選択 |
| 0xf1 | OUT | `Memory::outf1` | 辞書ROMバンク選択 |

**実装ファイル**: `src/pc88/memory.cpp`

**コメントから判明した詳細**:
- **Port31**: b2 b1 で N88/N80/RAM を選択
  - 0 0: N88
  - 1 0: N80
  - x 1: RAM
- **Port32**: b5=ALU Enable (port5x=RAM時), b4=RAM/~TVRAM, b1 b0=N88 EROM bank select
- **Port33**: b6=ALU Enable (port5x=RAM時)
- **Port34**: ALU演算設定
- **Port5c-5f**: GVRAM0-2, RAM選択
- **Port70**: TextWindow（N88時有効）
- **Port99**: b4=CD-BIOS/other
- **Porte2**: 拡張RAM制御
- **Porte4**: erom page select
- **Port F0**: 辞書ROMバンク選択

### 画面制御 (Screen)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0x30 | OUT | `Screen::out30` | CRTモードコントロール（b1: テキストサイズ） |
| 0x31 | OUT | `Screen::out31` | 画面表示制御（b4: color/bw, b3: graphic plane, b0: 200/400line） |
| 0x32 | OUT | `Screen::out32` | 画面制御 |
| 0x33 | OUT | `Screen::out33` | 画面制御 |
| 0x52 | OUT | `Screen::out52` | 画面制御 |
| 0x53 | OUT | `Screen::out53` | 画面制御 |
| 0x54 | OUT | `Screen::out54` | 画面制御 |
| 0x55-0x5b | OUT | `Screen::out55to5b` | 画面制御（パレット等） |

**実装ファイル**: `src/pc88/screen.cpp`

**コメントから判明した詳細**:
- **Out30**: b1=CRTモードコントロール
- **Out31**: b4=color/~b/w, b3=show graphic plane, b0=200line/~400line

### DMA制御 (PD8257)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0x60 | OUT | `PD8257::setaddr` | DMAチャネル0 アドレス設定 |
| 0x61 | OUT | `PD8257::setcount` | DMAチャネル0 カウント設定 |
| 0x62 | OUT | `PD8257::setaddr` | DMAチャネル1 アドレス設定 |
| 0x63 | OUT | `PD8257::setcount` | DMAチャネル1 カウント設定 |
| 0x64 | OUT | `PD8257::setaddr` | DMAチャネル2 アドレス設定 |
| 0x65 | OUT | `PD8257::setcount` | DMAチャネル2 カウント設定 |
| 0x66 | OUT | `PD8257::setaddr` | DMAチャネル3 アドレス設定 |
| 0x67 | OUT | `PD8257::setcount` | DMAチャネル3 カウント設定 |
| 0x68 | OUT | `PD8257::setmode` | DMAモード設定 |
| 0x60 | IN | `PD8257::getaddr` | DMAチャネル0 アドレス読み取り |
| 0x61 | IN | `PD8257::getcount` | DMAチャネル0 カウント読み取り |
| 0x62 | IN | `PD8257::getaddr` | DMAチャネル1 アドレス読み取り |
| 0x63 | IN | `PD8257::getcount` | DMAチャネル1 カウント読み取り |
| 0x64 | IN | `PD8257::getaddr` | DMAチャネル2 アドレス読み取り |
| 0x65 | IN | `PD8257::getcount` | DMAチャネル2 カウント読み取り |
| 0x66 | IN | `PD8257::getaddr` | DMAチャネル3 アドレス読み取り |
| 0x67 | IN | `PD8257::getcount` | DMAチャネル3 カウント読み取り |
| 0x68 | IN | `PD8257::getstat` | DMAステータス読み取り |

**実装ファイル**: `src/pc88/pd8257.cpp`

### 割り込み制御 (INTC)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0xe4 | OUT | `INTC::setreg` | 割り込みレジスタ設定 |
| 0xe6 | OUT | `INTC::setmask` | 割り込みマスク設定 |

**実装ファイル**: `src/pc88/intc.cpp`

**コメントから判明した詳細**:
- **0xe6**: マスク設定
- **0xe4**: レジスタ設定

### サブシステム通信 (SubSystem)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0xfc | OUT | `SubSystem::m_set0` | メイン側PIO Port0設定 |
| 0xfd | OUT | `SubSystem::m_set1` | メイン側PIO Port1設定 |
| 0xfe | OUT | `SubSystem::m_set2` | メイン側PIO Port2設定 |
| 0xff | OUT | `SubSystem::m_setcw` | メイン側PIO Control Word設定 |
| 0xfc | IN | `SubSystem::m_read0` | メイン側PIO Port0読み取り |
| 0xfd | IN | `SubSystem::m_read1` | メイン側PIO Port1読み取り |
| 0xfe | IN | `SubSystem::m_read2` | メイン側PIO Port2読み取り |

**実装ファイル**: `src/pc88/subsys.cpp`

**コメントから判明した詳細**:
- **0xfc-0xfe**: メイン側PIO（8255）のポート0-2
- **0xff**: メイン側PIOのControl Word
- PIOは8255のモード0のみエミュレート

### シリアルI/O (SIO) - テープ

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0x20 | OUT | `SIO::setdata` | シリアルデータ送信 |
| 0x21 | OUT | `SIO::setcontrol` | シリアル制御設定 |
| 0x20 | IN | `SIO::getdata` | シリアルデータ受信 |
| 0x21 | IN | `SIO::getstatus` | シリアルステータス読み取り |

**実装ファイル**: `src/pc88/sio.cpp`

**コメントから判明した詳細**:
- USART(uPD8251AF)の実装

### テープ管理 (TapeManager)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0x30 | OUT | `TapeManager::out30` | テープ制御 |
| 0x40 | IN | `TapeManager::in40` | テープ状態読み取り |

**実装ファイル**: `src/pc88/tapemgr.cpp`

### OPN音源 (OPNIF) - OPN1

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0x32 | OUT | `OPNIF::setintrmask` | 割り込みマスク設定 |
| 0x44 | OUT | `OPNIF::setindex0` | レジスタインデックス設定（チャネル0） |
| 0x45 | OUT | `OPNIF::writedata0` | レジスタデータ書き込み（チャネル0） |
| 0x46 | OUT | `OPNIF::setindex1` | レジスタインデックス設定（チャネル1） |
| 0x47 | OUT | `OPNIF::writedata1` | レジスタデータ書き込み（チャネル1） |
| 0x44 | IN | `OPNIF::readstatus` | ステータス読み取り |
| 0x45 | IN | `OPNIF::readdata0` | データ読み取り（チャネル0） |
| 0x46 | IN | `OPNIF::readstatusex` | 拡張ステータス読み取り |
| 0x47 | IN | `OPNIF::readdata1` | データ読み取り（チャネル1） |

**実装ファイル**: `src/pc88/opnif.cpp`

### OPN音源 (OPNIF) - OPN2

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0xaa | OUT | `OPNIF::setintrmask` | 割り込みマスク設定 |
| 0xa8 | OUT | `OPNIF::setindex0` | レジスタインデックス設定（チャネル0） |
| 0xa9 | OUT | `OPNIF::writedata0` | レジスタデータ書き込み（チャネル0） |
| 0xac | OUT | `OPNIF::setindex1` | レジスタインデックス設定（チャネル1） |
| 0xad | OUT | `OPNIF::writedata1` | レジスタデータ書き込み（チャネル1） |
| 0xa8 | IN | `OPNIF::readstatus` | ステータス読み取り |
| 0xa9 | IN | `OPNIF::readdata0` | データ読み取り（チャネル0） |
| 0xac | IN | `OPNIF::readstatusex` | 拡張ステータス読み取り |
| 0xad | IN | `OPNIF::readdata1` | データ読み取り（チャネル1） |

**実装ファイル**: `src/pc88/opnif.cpp`

### 漢字ROM (KanjiROM) - Kanji1

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0xe8 | OUT | `KanjiROM::setl` | 漢字コード下位バイト設定 |
| 0xe9 | OUT | `KanjiROM::seth` | 漢字コード上位バイト設定 |
| 0xe8 | IN | `KanjiROM::readl` | 漢字データ下位バイト読み取り |
| 0xe9 | IN | `KanjiROM::readh` | 漢字データ上位バイト読み取り |

**実装ファイル**: `src/pc88/kanjirom.cpp`

### 漢字ROM (KanjiROM) - Kanji2

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0xec | OUT | `KanjiROM::setl` | 漢字コード下位バイト設定 |
| 0xed | OUT | `KanjiROM::seth` | 漢字コード上位バイト設定 |
| 0xec | IN | `KanjiROM::readl` | 漢字データ下位バイト読み取り |
| 0xed | IN | `KanjiROM::readh` | 漢字データ上位バイト読み取り |

**実装ファイル**: `src/pc88/kanjirom.cpp`

### カレンダー (Calender)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0x10 | OUT | `Calender::out10` | カレンダー制御 |
| 0x40 | OUT | `Calender::out40` | カレンダー制御 |
| 0x40 | IN | `Calender::in40` | カレンダーデータ読み取り |

**実装ファイル**: `src/pc88/calender.cpp`

**注意**: 0x40は`Beep`と`Base`でも使用されています（優先順位による）

### ビープ音 (Beep)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0x40 | OUT | `Beep::out40` | ビープ音制御 |

**実装ファイル**: `src/pc88/beep.cpp`

**注意**: 0x40は`Calender`と`Base`でも使用されています（優先順位による）

### シリアルI/O (SIO) - MIDI

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0xc2 | OUT | `SIO::setdata` | MIDIデータ送信 |
| 0xc3 | OUT | `SIO::setcontrol` | MIDI制御設定 |
| 0xc2 | IN | `SIO::getdata` | MIDIデータ受信 |
| 0xc3 | IN | `SIO::getstatus` | MIDIステータス読み取り |

**実装ファイル**: `src/pc88/sio.cpp`

### ジョイパッド (JoyPad)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| `popnio` | IN | `JoyPad::getdir` | ジョイパッド方向読み取り |
| `popnio2` | IN | `JoyPad::getbutton` | ジョイパッドボタン読み取り |

**実装ファイル**: `src/pc88/joypad.cpp`

**注意**: `popnio`と`popnio2`は特殊ポート（0x100以上）で、OPNの入出力ポートと共用

---

## サブCPU側 I/Oポート

### サブシステム通信 (SubSystem)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0xfc | OUT | `SubSystem::s_set0` | サブ側PIO Port0設定 |
| 0xfd | OUT | `SubSystem::s_set1` | サブ側PIO Port1設定 |
| 0xfe | OUT | `SubSystem::s_set2` | サブ側PIO Port2設定 |
| 0xff | OUT | `SubSystem::s_setcw` | サブ側PIO Control Word設定 |
| 0xfc | IN | `SubSystem::s_read0` | サブ側PIO Port0読み取り |
| 0xfd | IN | `SubSystem::s_read1` | サブ側PIO Port1読み取り |
| 0xfe | IN | `SubSystem::s_read2` | サブ側PIO Port2読み取り |

**実装ファイル**: `src/pc88/subsys.cpp`

**コメントから判明した詳細**:
- **0xfc-0xfe**: サブ側PIO（8255）のポート0-2
- **0xff**: サブ側PIOのControl Word

### フロッピーディスク制御 (FDC)

| ポート | 方向 | 関数 | 説明 |
|------|------|------|------|
| 0xf4 | OUT | `FDC::drivecontrol` | ドライブ制御 |
| 0xf8 | OUT | `FDC::motorcontrol` | モーター制御 |
| 0xf8 | IN | `FDC::tcin` | トラック変更検出 |
| 0xfa | IN | `FDC::getstatus` | FDCステータス読み取り |
| 0xfb | OUT | `FDC::setdata` | FDCデータ書き込み |
| 0xfb | IN | `FDC::getdata` | FDCデータ読み取り |

**実装ファイル**: `src/pc88/fdc.cpp`

**コメントから判明した詳細**:
- **0xfa**: RQM DIO NDM CB D3B D2B D1B D0B
  - DIO = 0 なら CPU->FDC (Put), 1 なら FDC->CPU (Get)

---

## 特殊ポート（仮想ポート）

`PC88::SpecialPort` enumで定義されている特殊ポート（0x100以上）:

| ポート | 値 | 説明 |
|--------|-----|------|
| `pint0` | 0x100 | 割り込み要求0 |
| `pint1` | 0x101 | 割り込み要求1 |
| `pint2` | 0x102 | 割り込み要求2 |
| `pint3` | 0x103 | 割り込み要求3 |
| `pint4` | 0x104 | 割り込み要求4 |
| `pint5` | 0x105 | 割り込み要求5 |
| `pint6` | 0x106 | 割り込み要求6 |
| `pint7` | 0x107 | 割り込み要求7 |
| `pres` | 0x108 | リセット |
| `pirq` | 0x109 | IRQ |
| `piack` | 0x10a | 割り込み応答 |
| `vrtc` | 0x10b | 垂直帰線 |
| `popnio` | 0x10c | OPNの入出力ポート1 |
| `popnio2` | 0x10d | OPNの入出力ポート2（連番） |
| `psioin` | 0x10e | SIO関係 |
| `psioreq` | 0x10f | SIO要求 |
| `ptimesync` | 0x110 | タイムシンク |

**定義場所**: `src/pc88/pc88.h` (118-131行目)

---

## ポート競合について

一部のポートは複数のデバイスで使用されています。IOBusの接続順序により、先に接続されたデバイスが優先されます。

### 0x30
- **IN**: `Base::in30` (DIPSW読み取り)
- **OUT**: `Screen::out30` (画面制御), `TapeManager::out30` (テープ制御)

### 0x31
- **IN**: `Base::in31` (DIPSW読み取り)
- **OUT**: `Memory::out31` (メモリバンク選択), `Screen::out31` (画面制御)

### 0x40
- **IN**: `Base::in40` (システムステータス), `TapeManager::in40` (テープ状態), `Calender::in40` (カレンダー)
- **OUT**: `Calender::out40` (カレンダー制御), `Beep::out40` (ビープ音)

### 0x32
- **OUT**: `Memory::out32` (メモリバンク選択), `OPNIF::setintrmask` (OPN1割り込みマスク)
- **IN**: `Memory::in32` (メモリ状態)

### 0x33
- **OUT**: `Memory::out33` (メモリバンク選択), `CRTC::setkanamode` (かなモード)
- **IN**: `Memory::in33` (メモリ状態)

---

## 参考情報

- **ポート定義の場所**: `src/pc88/pc88.cpp` の `PC88::ConnectDevices()` 関数（284-583行目）
- **サブCPU側ポート定義**: `src/pc88/pc88.cpp` の `PC88::ConnectDevices2()` 関数（588-630行目）
- **特殊ポート定義**: `src/pc88/pc88.h` の `PC88::SpecialPort` enum（118-131行目）

---

## 注意事項

1. このドキュメントは、ソースコードのコメントと関数名から推測した情報に基づいています。
2. 実際のPC-8801ハードウェアの仕様とは異なる場合があります。
3. ポートの詳細な動作については、各デバイスクラスの実装ファイル（`src/pc88/*.cpp`）を参照してください。

