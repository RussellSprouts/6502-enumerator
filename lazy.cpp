template<typename t>
struct lazy {
  typedef t(*ptr)();

  ptr getter;
  t value;
  bool hasValue;

  explicit lazy(ptr getter): getter(getter), hasValue(false) {}

  t get() {
    if (hasValue) { return value; }
    else {
      value = getter();
      hasValue = true;
      return value;
    }
  }
};