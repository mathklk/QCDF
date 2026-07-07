
from dataclasses import dataclass
from matplotlib.axes import Axes
import numpy as np
import pandas as pd
from typing import *

FIG_SIZE = np.array([8, 5])

@dataclass
class Scenario:
    type: str
    distance: float
    data: str
    color: str

    df: pd.DataFrame = None # type: ignore

    def name(self):
        return f"{self.type} {self.distance}m"
    

def make_x_angle(ax: Axes, angles: List | pd.Index):
    ax.set_xticks(angles)
    ax.set_xlim(min(angles), max(angles))

def make_y_angle(ax: Axes, angles: List | pd.Index):
    ax.set_yticks(angles)
    ax.set_ylim(min(angles), max(angles))

def pscatter(ax: Axes, Z: pd.Series | pd.DataFrame, **kwargs):
    ax.scatter(Z.index, Z.to_numpy(), **kwargs)

def wrapplot(ax: Axes, Z: pd.Series | pd.DataFrame, low = -180, high = 180, **kwargs):
    pieces: Iterable[pd.Series] = [pd.Series(dtype=Z.dtype)]
    wrapRange = high - low
    prev = None
    for i in range(len(Z)):
        x = Z.index[i]
        y = Z.to_numpy()[i]
        if np.isnan(y):
            continue
        if prev is None:
            pieces[-1].loc[x] = y
        else:
            if abs(y - prev[1]) <= wrapRange / 2:
                pieces[-1].loc[x] = y
            else:
                pieces[-1].loc[x] = y - wrapRange * np.sign(y - prev[1])
                pieces.append(pd.Series(dtype=Z.dtype))
                pieces[-1].loc[prev[0]] = prev[1] + wrapRange * np.sign(y - prev[1])
                pieces[-1].loc[x] = y
        prev = (x, y)
    for piece in pieces:
        ax.plot(piece.index, piece.to_numpy(), **kwargs)
        if 'label' in kwargs:
            del kwargs['label']  # only add label once, or it will appear multiple times in the legend

def scatterandwrapplot(ax: Axes, Z: pd.Series | pd.DataFrame, **kwargs):
    wrapplot(ax, Z, **kwargs)
    if 'label' in kwargs:
        del kwargs['label']
    pscatter(ax, Z, **kwargs)

def legend(ax: Axes, **kwargs):
    l = ax.legend(**kwargs)
    l.get_frame().set_alpha(1.0)