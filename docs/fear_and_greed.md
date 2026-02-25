# Fear & Greed Index API Reference

Fetch the CNN Fear & Greed Index, a sentiment indicator for the U.S. stock market.

## Usage

```cpp
auto data = yFinance::getFearAndGreedIndex();
```

No parameters required.

## Response: `FearAndGreedInfo`

### Current Data

| Field | Type | Description |
|-------|------|-------------|
| `score` | `double` | Current index score (0–100) |
| `rating` | `string` | Sentiment label |
| `timestamp` | `string` | Observation time (e.g., `"2026-02-20 01:00:00"`) |
| `previousClose` | `double` | Previous close score |
| `previousWeek` | `double` | Score 1 week ago |
| `previousMonth` | `double` | Score 1 month ago |
| `previousYear` | `double` | Score 1 year ago |

### Historical Data

| Field | Type | Description |
|-------|------|-------------|
| `timestamps` | `vector<int64_t>` | Historical timestamps (Unix) |
| `scores` | `vector<double>` | Historical scores (0–100) |
| `ratings` | `vector<string>` | Historical sentiment labels |

### Rating Values

| Score Range | Rating |
|-------------|--------|
| 0 – 24 | Extreme Fear |
| 25 – 44 | Fear |
| 45 – 55 | Neutral |
| 56 – 75 | Greed |
| 76 – 100 | Extreme Greed |
