# okec::simulator

*Defined in header `<okec/common/simulator.h>`*<br>
*Defined in namespace okec*

```cpp
class simulator {};
```

## Member functions

|||
|-------------|--------|
|[(constructor)](../simulator/simulator)|constructs a new simulator<br><span style="color: green">(public member function)</span>|
|[(destructor)](../simulator/~simulator) |destructs the simulator<br><span style="color: green">(public member function)|
|[run](../simulator/run)|runs the simulator<br><span style="color: green">(public member function)|
|[stop_time (getter)](../simulator/stop_time)|gets the stop time of the simulator<br><span style="color: green">(public member function)|
|[stop_time (setter)](#stop_time-setter)|sets the stop time of the simulator<br><span style="color: green">(public member function)|
|[submit](../simulator/submit)|sets the coroutine resume function<br><span style="color: green">(public member function)|
|[complete](../simulator/complete)|invokes the resume function when the response is arrived<br><span style="color: green">(public member function)|
|[is_valid](../simulator/is_valid)|checks if the ip address has a resume function<br><span style="color: green">(public member function)|
|[hold_coro](../simulator/hold_coro)|holds a awaitable object in case it destroyed<br><span style="color: green">(public member function)|



### stop_time setter
```cpp
auto stop_time() const -> ns3::Time;
```

## Notes

## Example