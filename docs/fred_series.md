# FRED Economic Data Series Reference

A reference of key U.S. macroeconomic indicators available via the FRED (Federal Reserve Economic Data) API.

> Get your free API key at: https://fred.stlouisfed.org/docs/api/api_key.html

## Usage

```cpp
auto data = yFinance::getFredSeries("UNRATE", apiKey, "2025-01-01", "2026-02-01", "m");
```

| Parameter | Description |
|-----------|-------------|
| `seriesId` | FRED series ID |
| `apiKey` | FRED API key |
| `observationStart` | Start date (YYYY-MM-DD), optional |
| `observationEnd` | End date (YYYY-MM-DD), optional |
| `frequency` | `"d"`, `"w"`, `"m"`, `"q"`, `"a"` — auto fallback if unsupported |

---

## 💼 Employment / Monetary Policy

| Series ID | Description | Frequency |
|-----------|-------------|-----------|
| `UNRATE` | U.S. Unemployment Rate | Monthly |
| `FEDFUNDS` | Federal Funds Effective Rate | Monthly |
| `PAYEMS` | Nonfarm Payrolls (NFP) | Monthly |

## 💵 Money Supply / Debt / Exchange Rate

| Series ID | Description | Frequency |
|-----------|-------------|-----------|
| `WM2NS` | M2 Money Supply | Monthly |
| `M2REAL` | Real M2 Money Supply | Monthly |
| `GFDEBTN` | Federal Government Total Debt | Quarterly |
| `DEXKOUS` | KRW/USD Exchange Rate | Daily |

## 📊 Business Cycle / Growth

| Series ID | Description | Frequency |
|-----------|-------------|-----------|
| `GDP` | U.S. Gross Domestic Product | Quarterly |
| `GDPC1` | Real GDP | Quarterly |
| `INDPRO` | Industrial Production Index | Monthly |

## 📈 Inflation

| Series ID | Description | Frequency |
|-----------|-------------|-----------|
| `CPIAUCSL` | Consumer Price Index (CPI) | Monthly |
| `CPILFESL` | Core CPI (excl. Food & Energy) | Monthly |
| `PCEPI` | PCE Price Index (Fed preferred) | Monthly |
| `T10YIE` | 10-Year Breakeven Inflation Rate | Daily |

## 💰 Interest Rates / Bonds

| Series ID | Description | Frequency |
|-----------|-------------|-----------|
| `DGS10` | 10-Year Treasury Yield | Daily |
| `DGS2` | 2-Year Treasury Yield | Daily |
| `T10Y2Y` | 10Y-2Y Yield Spread (recession signal) | Daily |
| `BAMLH0A0HYM2` | High Yield Spread (credit risk) | Daily |

## 🏠 Consumer / Housing

| Series ID | Description | Frequency |
|-----------|-------------|-----------|
| `UMCSENT` | U. of Michigan Consumer Sentiment | Monthly |
| `HOUST` | Housing Starts | Monthly |
| `RSAFS` | Retail Sales | Monthly |
