from django.db import models


class Entry(models.Model):
    created = models.DateTimeField(auto_now_add=True)
    kind = models.CharField(max_length=100, blank=True, default='')
    value = models.CharField(max_length=100, blank=True, default='')

    class Meta:
        ordering = ['created']

