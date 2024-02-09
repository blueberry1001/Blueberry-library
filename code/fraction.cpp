//参考
//https://qiita.com/gandalfr-KY/items/b0eed6727e873a313455
#include<bits/stdc++.h>
__int128_t __gcd(__int128_t a, __int128_t b) {
  if (a % b == 0)
    return b;
  return __gcd(b, a % b);
}
__int128_t Gcd(__int128_t a, __int128_t b) {
  if (b == 0)
    return abs(a);
  return __gcd(abs(a), abs(b));
}
inline void simplify(__int128_t &num, __int128_t &den) {
  __int128_t d = Gcd(num, den);
  num /= (den >= 0 ? d : -d);
  den /= (den >= 0 ? d : -d);
}

class fraction{
	public:
	ll num,den;
	fraction(long long n) : num(n), den(1) {}
	fraction(__int128_t a,__int128_t b){
		simplify(a,b);
		num = a,den = b;
	}
	fraction() : num(0), den(1) {}
	friend fraction operator+(const fraction &a) { return a; }
  friend fraction operator-(const fraction &a) {
    fraction ret;
    ret.raw_assign(-a.num, a.den);
    return ret;
  }
	fraction &raw_assign(long long _num, long long _den) {
        num = _num, den = _den;
        return *this;
    }
	friend fraction operator+(const fraction &a, const fraction &b) {
        return {(__int128_t)a.num * b.den + (__int128_t)b.num * a.den,
                (__int128_t)a.den * b.den};
    }
    friend fraction operator-(const fraction &a, const fraction &b) {
        return {(__int128_t)a.num * b.den - (__int128_t)b.num * a.den,
                (__int128_t)a.den * b.den};
    }
    friend fraction operator*(const fraction &a, const fraction &b) {
    __int128_t gcd_tmp1 = std::gcd(a.num, b.den),
               gcd_tmp2 = std::gcd(b.num, a.den);
    return {(a.num / gcd_tmp1) * (b.num / gcd_tmp2),
            (a.den / gcd_tmp2) * (b.den / gcd_tmp1)};
  }
    friend fraction operator/(const fraction &a, const fraction &b) {
    __int128_t gcd_tmp1 = std::gcd(a.num, b.num),
               gcd_tmp2 = std::gcd(b.den, a.den);
    return { (a.num / gcd_tmp1) * (b.den / gcd_tmp2),
             (a.den / gcd_tmp2) * (b.num / gcd_tmp1),
            };
  	}
	friend bool operator==(const fraction &a, const fraction &b) {
    return a.num == b.num && a.den == b.den;
  }
  friend bool operator!=(const fraction &a, const fraction &b) {
    return a.num != b.num || a.den != b.den;
  }
  friend bool operator>(const fraction &a, const fraction &b) {
    return (__int128_t)a.num * b.den > (__int128_t)b.num * a.den;
  }
  friend bool operator>=(const fraction &a, const fraction &b) {
    return (__int128_t)a.num * b.den >= (__int128_t)b.num * a.den;
  }
  friend bool operator<(const fraction &a, const fraction &b) {
    return (__int128_t)a.num * b.den < (__int128_t)b.num * a.den;
  }
  friend bool operator<=(const fraction &a, const fraction &b) {
    return (__int128_t)a.num * b.den <= (__int128_t)b.num * a.den;
  }
  fraction &operator=(const fraction &a) = default;
  fraction &operator+=(const fraction &a) { return *this = *this + a; }
  fraction &operator-=(const fraction &a) { return *this = *this - a; }
  fraction &operator*=(const fraction &a) { return *this = *this * a; }
  fraction &operator/=(const fraction &a) { return *this = *this / a; }
  friend std::ostream &operator<<(std::ostream &os, const fraction &a) {
    if (a.den == 1)
      os << a.num;
    else
      os << a.num << '/' << a.den;
    return os;
  }
};
