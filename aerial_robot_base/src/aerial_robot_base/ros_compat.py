#!/usr/bin/env python3

import threading
import time
from typing import Any, Dict, Tuple

import rclpy
from rclpy.duration import Duration as RclpyDuration
from rclpy.executors import MultiThreadedExecutor
from rclpy.node import Node
from rclpy.time import Time as RclpyTime

_node = None
_executor = None
_spin_thread = None
_declared_params = set()
_throttle_state: Dict[Tuple[str, str], float] = {}


class ServiceException(Exception):
    pass


def _normalize_param_name(name: str) -> str:
    param_name = name
    if param_name.startswith("~"):
        param_name = param_name[1:]
    param_name = param_name.strip("/")
    return param_name.replace("/", ".") if param_name else "anonymous_param"


def _start_background_spin() -> None:
    global _spin_thread

    if _spin_thread is not None and _spin_thread.is_alive():
        return

    def _spin_loop() -> None:
        try:
            _executor.spin()
        except Exception:
            pass

    _spin_thread = threading.Thread(target=_spin_loop, daemon=True)
    _spin_thread.start()


def _ensure_node(node_name: str = "aerial_robot_base_py") -> Node:
    global _node, _executor

    if not rclpy.ok():
        rclpy.init(args=None)

    if _node is None:
        _node = rclpy.create_node(node_name)
        _executor = MultiThreadedExecutor(num_threads=4)
        _executor.add_node(_node)
        _start_background_spin()

    return _node


def get_node() -> Node:
    return _ensure_node()


def init_node(name: str) -> Node:
    return _ensure_node(name)


def is_shutdown() -> bool:
    return not rclpy.ok()


def shutdown() -> None:
    global _node, _executor

    if _executor is not None and _node is not None:
        try:
            _executor.remove_node(_node)
        except Exception:
            pass
    if _node is not None:
        _node.destroy_node()
        _node = None
    if rclpy.ok():
        rclpy.shutdown()
    _executor = None


def get_param(name: str, default: Any = None) -> Any:
    node = _ensure_node()
    param_name = _normalize_param_name(name)

    if param_name not in _declared_params:
        node.declare_parameter(param_name, default)
        _declared_params.add(param_name)

    return node.get_parameter(param_name).value


def Publisher(topic: str, msg_type: Any, queue_size: int = 10):
    return _ensure_node().create_publisher(msg_type, topic, queue_size)


def Subscriber(topic: str, msg_type: Any, callback: Any, queue_size: int = 10):
    return _ensure_node().create_subscription(msg_type, topic, callback, queue_size)


class _ServiceProxy:
    def __init__(self, topic: str, srv_type: Any):
        self.node = _ensure_node()
        self.client = self.node.create_client(srv_type, topic)

    def __call__(self, request: Any) -> Any:
        if not self.client.wait_for_service(timeout_sec=2.0):
            raise ServiceException(f"Service unavailable: {self.client.srv_name}")

        future = self.client.call_async(request)
        while rclpy.ok() and not future.done():
            time.sleep(0.01)

        if future.cancelled():
            raise ServiceException("Service call cancelled")
        if future.exception() is not None:
            raise ServiceException(str(future.exception()))

        return future.result()


def ServiceProxy(topic: str, srv_type: Any):
    return _ServiceProxy(topic, srv_type)


def sleep(duration_sec: float) -> None:
    time.sleep(duration_sec)


def get_time() -> float:
    return _ensure_node().get_clock().now().nanoseconds * 1e-9


class _TimeProxy:
    @staticmethod
    def now():
        return _ensure_node().get_clock().now().to_msg()


Time = _TimeProxy


class _DurationProxy:
    def __new__(cls, sec: float = 0.0):
        return RclpyDuration(seconds=sec)


Duration = _DurationProxy


def _log(level: str, msg: str) -> None:
    logger = _ensure_node().get_logger()
    if level == "debug":
        logger.debug(msg)
    elif level == "warn":
        logger.warning(msg)
    elif level == "error":
        logger.error(msg)
    else:
        logger.info(msg)


def loginfo(msg: str, *args: Any) -> None:
    _log("info", msg % args if args else msg)


def logwarn(msg: str, *args: Any) -> None:
    _log("warn", msg % args if args else msg)


def logerr(msg: str, *args: Any) -> None:
    _log("error", msg % args if args else msg)


def logdebug(msg: str, *args: Any) -> None:
    _log("debug", msg % args if args else msg)


def _throttled(level: str, period_sec: float, msg: str) -> None:
    now = get_time()
    key = (level, msg)
    last = _throttle_state.get(key)
    if last is None or (now - last) >= period_sec:
        _throttle_state[key] = now
        _log(level, msg)


def loginfo_throttle(period_sec: float, msg: str) -> None:
    _throttled("info", period_sec, msg)


def logdebug_throttle(period_sec: float, msg: str) -> None:
    _throttled("debug", period_sec, msg)


def logwarn_throttle(period_sec: float, msg: str) -> None:
    _throttled("warn", period_sec, msg)
