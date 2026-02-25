# Stock Data API Reference

Fetch historical stock data from Yahoo Finance.

## Usage

### By Range

```cpp
auto data = yFinance::getStockInfo("AAPL", "1d", "1mo");
```

| Parameter | Description | Default |
|-----------|-------------|---------|
| `ticker` | Stock ticker symbol (e.g., `"AAPL"`, `"^IXIC"`, `"005930.KS"`) | required |
| `interval` | Data interval | `"1d"` |
| `range` | Date range | `"1mo"` |

### By Date Range

```cpp
auto data = yFinance::getStockInfo("AAPL", "2025-01-01", "2025-02-01", "1d");
```

| Parameter | Description |
|-----------|-------------|
| `ticker` | Stock ticker symbol |
| `startDate` | Start date (YYYY-MM-DD) |
| `endDate` | End date (YYYY-MM-DD) |
| `interval` | Data interval |

## Interval Values

| Value | Description |
|-------|-------------|
| `"1d"` | Daily |
| `"1wk"` | Weekly |
| `"1mo"` | Monthly |

## Range Values

| Value | Description |
|-------|-------------|
| `"1d"`, `"5d"` | 1 / 5 days |
| `"1mo"`, `"3mo"`, `"6mo"` | 1 / 3 / 6 months |
| `"1y"`, `"2y"`, `"5y"`, `"10y"` | 1 / 2 / 5 / 10 years |
| `"ytd"` | Year to date |
| `"max"` | Maximum available |

## Response: `StockInfo`

| Field | Type | Description |
|-------|------|-------------|
| `ticker` | `string` | Ticker symbol |
| `currency` | `string` | Currency (e.g., `"USD"`, `"KRW"`) |
| `exchangeName` | `string` | Exchange (e.g., `"NMS"`, `"KSC"`) |
| `instrumentType` | `string` | Type (e.g., `"EQUITY"`, `"ETF"`) |
| `timezone` | `string` | Timezone (e.g., `"America/New_York"`) |
| `regularMarketPrice` | `double` | Current market price |
| `chartPreviousClose` | `double` | Previous close price |
| `firstTradeDate` | `int64_t` | First trade date (Unix timestamp) |
| `gmtoffset` | `int32_t` | GMT offset in seconds |
| `timestamps` | `vector<int64_t>` | Historical timestamps |
| `open` | `vector<double>` | Open prices |
| `high` | `vector<double>` | High prices |
| `low` | `vector<double>` | Low prices |
| `close` | `vector<double>` | Close prices |
| `volume` | `vector<int64_t>` | Trading volumes |
