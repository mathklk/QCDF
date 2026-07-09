
from dataclasses import dataclass
from matplotlib.axes import Axes
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from typing import *

FIG_SIZE = np.array([8, 5])

@dataclass
class Scenario:
    type: str
    distance: float
    data: str

    df: pd.DataFrame = None # type: ignore

    def color(self):
        _c = {
            ('ULA', 3.5): "#004c6d",
            ('ULA', 20 ): "#6698b3",
            ('ULA', 100): '#beebff',
            ('UCA', 20 ): '#ed7d31',
        }
        return _c.get((self.type, self.distance))

    def name(self):
        return f"{self.type} {self.distance}m"
    
uca_colors = [
    "#ff8629",
    "#bd4715",
    "#790000",
]

def subplots(nrows: int = 1, ncols: int = 1, **kwargs):
    default = {
        'figsize': FIG_SIZE * np.array([ncols, nrows]),
    }
    return plt.subplots(nrows=nrows, ncols=ncols, **{**default, **kwargs}) # type: ignore

def make_x_angle(ax: Axes, angles: Iterable):
    ax.set_xticks(list(angles))
    ax.set_xticklabels([str(a) for a in angles])
    ax.set_xlim(min(angles), max(angles))

def make_y_angle(ax: Axes, angles: Iterable):
    ax.set_yticks(list(angles))
    ax.set_ylim(min(angles), max(angles))

def pscatter(ax: Axes, Z: pd.Series, **kwargs):
    ax.scatter(Z.index, Z.to_numpy(), **kwargs)

def wrapsegments(Z: pd.Series, low = -180, high = 180) -> List[pd.Series]:
    pieces: list[pd.Series] = [pd.Series(dtype=Z.dtype)]
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
    return pieces

def wrapplot(ax: Axes, Z: pd.Series, low = -180, high = 180, **kwargs):
    for piece in wrapsegments(Z, low, high):
        ax.plot(piece.index, piece.to_numpy(), **kwargs)
        if 'label' in kwargs:
            del kwargs['label']  # only add label once, or it will appear multiple times in the legend

def scatterandwrapplot(ax: Axes, Z: pd.Series, **kwargs):
    wrapplot(ax, Z, **kwargs)
    if 'label' in kwargs:
        del kwargs['label']
    pscatter(ax, Z, **kwargs)

def wrapfillmeanstd(ax: Axes, mean: pd.Series, std: pd.Series, sigma, low = -180, high = 180, **kwargs):
    for piece in wrapsegments(mean, low, high):
        ax.fill_between(piece.index, piece - sigma*std.loc[piece.index], piece + sigma*std.loc[piece.index], **kwargs)
        if 'label' in kwargs:
            del kwargs['label']  # only add label once, or it will appear multiple times in the legend

def wrapfillquantiles(ax: Axes, median: pd.Series, lowerQ: pd.Series, upperQ: pd.Series, low = -180, high = 180, **kwargs):
    relativeUpperQ = upperQ - median
    relativeLowerQ = median - lowerQ
    for piece in wrapsegments(median, low, high):
        # ax.fill_between(piece.index, lowerQ.loc[piece.index], upperQ.loc[piece.index], **kwargs)
        ax.fill_between(piece.index, piece.loc[piece.index] - relativeLowerQ.loc[piece.index], piece.loc[piece.index] + relativeUpperQ.loc[piece.index], **kwargs)
        if 'label' in kwargs:
            del kwargs['label']  # only add label once, or it will appear multiple times in the legend

def legend(ax: Axes, **kwargs):
    l = ax.legend(**kwargs)
    l.get_frame().set_alpha(1.0)