from django.db import models


class Alert(models.Model):
    created = models.DateTimeField(auto_now_add=True)
    message = models.CharField(max_length=100, blank=True, default='')
    owner = models.ForeignKey('auth.User', related_name='alerts',
                              on_delete=models.CASCADE)

    class Meta:
        ordering = ['created']

    def highlight(self):
        return f"<b>{self.created}:</b> {self.message}"

    def save(self, *args, **kwargs):
        """
        """
        self.highlighted = self.highlight()
        super(Alert, self).save(*args, **kwargs)


class Entry(models.Model):
    created = models.DateTimeField(auto_now_add=True)
    value = models.CharField(max_length=100, blank=True, default='')
    raw = models.IntegerField(default=-1)
    owner = models.ForeignKey('auth.User', related_name='entries',
                              on_delete=models.CASCADE)
    highlighted = models.TextField(default='')

    class Meta:
        ordering = ['created']

    def highlight(self):
        return f"<b>{self.created}:</b> {self.value}"

    def save(self, *args, **kwargs):
        """
        """
        self.highlighted = self.highlight()
        super(Entry, self).save(*args, **kwargs)

# 2021-04-28T19:13:09.025559Z
class PumpingEntry(models.Model):
    created = models.DateTimeField(auto_now_add=True)
    value = models.IntegerField()
    owner = models.ForeignKey('auth.User', related_name='pumping_entries',
                              on_delete=models.CASCADE)

    class Meta:
        ordering = ['created']

    def highlight(self):
        return f"<b>{self.created}:</b> {self.value}"
