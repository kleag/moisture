from django.db import models


class Entry(models.Model):
    created = models.DateTimeField(auto_now_add=True)
    kind = models.CharField(max_length=100, blank=True, default='')
    value = models.CharField(max_length=100, blank=True, default='')
    owner = models.ForeignKey('auth.User', related_name='entries',
                              on_delete=models.CASCADE)
    highlighted = models.TextField(default='')

    class Meta:
        ordering = ['created']

    def highlight(self):
        return f"<b>{self.created}:</b> {self.kind} - {self.value}"

    def save(self, *args, **kwargs):
        """
        """
        self.highlighted = self.highlight()
        super(Entry, self).save(*args, **kwargs)
