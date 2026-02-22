# Window Functions (23 Functions)

Full-scan window functions. Each function returns one value per row. Do not use `GROUP BY`; calculations are performed within the window frame.

← [Function Reference](../function_reference.md)

---

## Rolling Statistics

> **Note**: All functions in this category are implemented as **window functions** (full-scan type). Each returns one value per row. Do not use `GROUP BY`.

### stat_rolling_mean

Computes the **rolling mean** (moving average). Returns the average of the most recent n values within the specified window size for each row.

**Syntax**: `stat_rolling_mean(column, window_size)`

| Parameter | Description |
|---|---|
| `window_size` | Window size (integer) |

> Leading rows with fewer values than the window size return `NULL`.

```sql
-- 3-day rolling mean
SELECT date, price,
       stat_rolling_mean(price, 3) AS rolling_avg
FROM stock_prices;

-- Smoothing with a 7-day moving average
SELECT timestamp, value,
       stat_rolling_mean(value, 7) AS smoothed
FROM sensor_data;
```

---

### stat_rolling_std

Computes the **rolling standard deviation**.

**Syntax**: `stat_rolling_std(column, window_size)`

```sql
SELECT date, price,
       stat_rolling_std(price, 5) AS rolling_volatility
FROM stock_prices;
```

---

### stat_rolling_min

Computes the **rolling minimum**.

**Syntax**: `stat_rolling_min(column, window_size)`

```sql
SELECT date, price,
       stat_rolling_min(price, 20) AS min_20d
FROM stock_prices;
```

---

### stat_rolling_max

Computes the **rolling maximum**.

**Syntax**: `stat_rolling_max(column, window_size)`

```sql
SELECT date, price,
       stat_rolling_max(price, 20) AS max_20d
FROM stock_prices;
```

---

### stat_rolling_sum

Computes the **rolling sum**.

**Syntax**: `stat_rolling_sum(column, window_size)`

```sql
-- 7-day cumulative sales
SELECT date, sales,
       stat_rolling_sum(sales, 7) AS weekly_total
FROM daily_sales;
```

---

## Moving Averages

### stat_moving_avg

Computes the **simple moving average** (SMA). Equivalent to `stat_rolling_mean`.

**Syntax**: `stat_moving_avg(column, window_size)`

```sql
SELECT date, close_price,
       stat_moving_avg(close_price, 20) AS sma_20
FROM stocks;
```

---

### stat_ema

Computes the **exponential moving average** (EMA). A moving average that places greater weight on more recent data.

$$\text{EMA}_t = \alpha \cdot x_t + (1-\alpha) \cdot \text{EMA}_{t-1}, \quad \alpha = \frac{2}{\text{span}+1}$$

**Syntax**: `stat_ema(column, span)`

| Parameter | Description |
|---|---|
| `span` | Span (integer). Same convention as pandas `ewm(span=N)` |

```sql
-- 12-day EMA (alpha = 2/13 ≈ 0.154)
SELECT date, close_price,
       stat_ema(close_price, 12) AS ema_12
FROM stocks;

-- Comparing SMA and EMA
SELECT date, close_price,
       stat_moving_avg(close_price, 20) AS sma_20,
       stat_ema(close_price, 20) AS ema_20
FROM stocks;
```

---

## Rank

### stat_rank

Performs **rank transformation**. Returns the rank for each row's value. Tied values receive the average rank. NULL rows return NULL.

**Syntax**: `stat_rank(column)`

```sql
-- Ranking scores
SELECT name, score,
       stat_rank(score) AS rank
FROM students;
```

---

## Missing Value Imputation

### stat_fillna_mean

Fills NULL values with the **mean of non-NULL values**.

**Syntax**: `stat_fillna_mean(column)`

```sql
SELECT id, value,
       stat_fillna_mean(value) AS filled
FROM measurements;
```

---

### stat_fillna_median

Fills NULL values with the **median of non-NULL values**.

**Syntax**: `stat_fillna_median(column)`

```sql
SELECT id, value,
       stat_fillna_median(value) AS filled
FROM measurements;
```

---

### stat_fillna_ffill

Fills NULL values with the **most recent non-NULL value** (forward fill). Leading NULLs remain NULL.

**Syntax**: `stat_fillna_ffill(column)`

```sql
-- Forward fill for time series data
SELECT timestamp, sensor_value,
       stat_fillna_ffill(sensor_value) AS filled
FROM sensor_data;
```

---

### stat_fillna_bfill

Fills NULL values with the **next non-NULL value** (backward fill). Trailing NULLs remain NULL.

**Syntax**: `stat_fillna_bfill(column)`

```sql
SELECT timestamp, sensor_value,
       stat_fillna_bfill(sensor_value) AS filled
FROM sensor_data;
```

---

### stat_fillna_interp

Fills NULL values using **linear interpolation**. Linearly interpolates between the surrounding non-NULL values.

**Syntax**: `stat_fillna_interp(column)`

```sql
-- Linear interpolation for time series missing values
SELECT timestamp, temperature,
       stat_fillna_interp(temperature) AS interpolated
FROM hourly_data;
```

---

## Encoding / Binning

### stat_label_encode

Performs **label encoding** on values. Sorts unique values and assigns integers starting from 0.

**Syntax**: `stat_label_encode(column)`

```sql
-- Convert categorical values to numeric
SELECT id, color,
       stat_label_encode(color) AS encoded
FROM products;
-- color=blue → 0, green → 1, red → 2 (sorted order)
```

---

### stat_bin_width

Performs **equal-width binning**. Divides the value range into n equal-width bins and returns the bin number (0 to n-1) each value belongs to.

**Syntax**: `stat_bin_width(column, n_bins)`

| Parameter | Description |
|---|---|
| `n_bins` | Number of bins (integer) |

```sql
-- Classify ages into 5 bins
SELECT name, age,
       stat_bin_width(age, 5) AS age_group
FROM customers;
```

---

### stat_bin_freq

Performs **equal-frequency binning** (quantile binning). Divides into n bins so that each bin contains approximately the same number of records.

**Syntax**: `stat_bin_freq(column, n_bins)`

| Parameter | Description |
|---|---|
| `n_bins` | Number of bins (integer) |

```sql
-- Binning based on quartiles
SELECT name, income,
       stat_bin_freq(income, 4) AS income_quartile
FROM households;
```

---

## Time Series Transformations

### stat_lag

Applies a **lag** (delay). Returns the value from k rows earlier. The first k rows return NULL.

**Syntax**: `stat_lag(column, k)`

| Parameter | Description |
|---|---|
| `k` | Lag amount (integer, positive value) |

```sql
-- Get the previous period's value
SELECT date, price,
       stat_lag(price, 1) AS prev_price,
       price - stat_lag(price, 1) AS price_change
FROM stock_prices;
```

---

### stat_diff

Computes **differencing**. Applies differencing of the specified order. The first `order` rows return NULL.

**Syntax**: `stat_diff(column, order)`

| Parameter | Description |
|---|---|
| `order` | Order of differencing (integer) |

```sql
-- First-order differencing (day-over-day change)
SELECT date, price,
       stat_diff(price, 1) AS daily_change
FROM stock_prices;

-- Second-order differencing (acceleration)
SELECT date, value,
       stat_diff(value, 2) AS acceleration
FROM time_series;
```

---

### stat_seasonal_diff

Computes **seasonal differencing**. Returns the difference from the value `period` rows earlier. The first `period` rows return NULL.

**Syntax**: `stat_seasonal_diff(column, period)`

| Parameter | Description |
|---|---|
| `period` | Seasonal period (integer) |

```sql
-- Year-over-year comparison (monthly data, period 12)
SELECT month, sales,
       stat_seasonal_diff(sales, 12) AS yoy_change
FROM monthly_sales;

-- Week-over-week same-day comparison (daily data, period 7)
SELECT date, visitors,
       stat_seasonal_diff(visitors, 7) AS wow_change
FROM daily_traffic;
```

---

## Outlier Detection

### stat_outliers_iqr

Detects outliers using the **IQR method**. Values below Q1 - 1.5*IQR or above Q3 + 1.5*IQR are flagged as outliers (1.0); all others are flagged as normal (0.0). NULL rows return NULL.

**Syntax**: `stat_outliers_iqr(column)`

```sql
-- Add outlier flags
SELECT id, value,
       stat_outliers_iqr(value) AS is_outlier
FROM measurements;

-- Mean excluding outliers
SELECT stat_mean(value) AS mean_without_outliers
FROM (
    SELECT value
    FROM measurements
    WHERE stat_outliers_iqr(value) = 0.0
);
```

---

### stat_outliers_zscore

Detects outliers using the **Z-score method**. Values with |z| > 3 are flagged as outliers (1.0).

**Syntax**: `stat_outliers_zscore(column)`

```sql
SELECT id, value,
       stat_outliers_zscore(value) AS is_outlier
FROM measurements;
```

---

### stat_outliers_mzscore

Detects outliers using the **modified Z-score method**. A robust method that uses the median and MAD. Values with |modified Z-score| > 3.5 are flagged as outliers (1.0).

**Syntax**: `stat_outliers_mzscore(column)`

```sql
-- Robust outlier detection
SELECT id, value,
       stat_outliers_mzscore(value) AS is_outlier
FROM measurements;

-- Comparing 3 methods
SELECT id, value,
       stat_outliers_iqr(value) AS iqr,
       stat_outliers_zscore(value) AS zscore,
       stat_outliers_mzscore(value) AS mzscore
FROM measurements;
```

---

## Robust Processing

### stat_winsorize

Performs **winsorization** (tail clipping). Clips extreme values at the specified percentile on both ends.

**Syntax**: `stat_winsorize(column, percentile)`

| Parameter | Description |
|---|---|
| `percentile` | Percentile for clipping (integer, e.g., 5 → clips at 5th and 95th percentiles) |

```sql
-- Winsorize at the 5th percentile
SELECT id, value,
       stat_winsorize(value, 5) AS winsorized
FROM measurements;

-- Statistics after winsorization
SELECT stat_mean(w) AS winsorized_mean
FROM (
    SELECT stat_winsorize(value, 10) AS w FROM measurements
);
```

---
